// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
#include "openkeyboardinputcontext.h"
#include "bridge/openkeyboardbridge.h"
#include <QGuiApplication>
#include <QScreen>
#include <QInputMethodEvent>
#include <QInputMethod>

OpenKeyboardInputContext::OpenKeyboardInputContext(QObject *parent)
    : QPlatformInputContext()
{
    Q_UNUSED(parent)
    auto *bridge = OpenKeyboardBridge::instance();
    connect(bridge, &OpenKeyboardBridge::commitRequested,
            this, &OpenKeyboardInputContext::onCommitRequested);
    connect(bridge, &OpenKeyboardBridge::deleteRequested,
            this, &OpenKeyboardInputContext::onDeleteRequested);
    connect(bridge, &OpenKeyboardBridge::hideRequested,
            this, &OpenKeyboardInputContext::onHideRequested);
}

OpenKeyboardInputContext::~OpenKeyboardInputContext() = default;

bool OpenKeyboardInputContext::isValid() const { return true; }

static bool isKeyboardChrome(QObject *object)
{
    for (QObject *cur = object; cur; cur = cur->parent()) {
        const QMetaObject *mo = cur->metaObject();
        const QByteArray cn = mo->className();
        if (cn.contains("KeyButton") || cn.contains("TextKey") || cn.contains("FunctionKey")
            || cn.contains("ActionKey") || cn.contains("CandidateBar")
            || cn.contains("OpenKeyboard") || cn.contains("OpenInputPanel") || cn.contains("ToolButton"))
            return true;
        if (mo->indexOfProperty("keyText") != -1 || mo->indexOfProperty("keyChar") != -1)
            return true;
        // OpenKeyboard / OpenInputPanel roots are plain QQuickRectangle/QQuickItem but expose unique properties
        if (mo->indexOfProperty("showCandidateBar") != -1 || mo->indexOfProperty("actionLabel") != -1)
            return true;
    }
    return false;
}

static bool isTextInput(QObject *object)
{
    return object && object->metaObject()->indexOfProperty("inputMethodHints") != -1;
}

void OpenKeyboardInputContext::setFocusObject(QObject *object)
{
    if (object && isKeyboardChrome(object))
        return;
    if (object && !isTextInput(object)) {
        if (m_visible)
            hideInputPanel();
        if (m_focusObject) {
            m_focusObject = nullptr;
            QPlatformInputContext::setFocusObject(nullptr);
        }
        return;
    }
    if (m_focusObject == object)
        return;
    m_focusObject = object;
    if (object) {
        queryFocusObject();
        showInputPanel();
    } else {
        hideInputPanel();
    }
    QPlatformInputContext::setFocusObject(object);
    // After Qt wires the new focus object, re-query so surrounding text etc.
    // are read from the *new* focus item (first show included).
    queryFocusObject();
}

void OpenKeyboardInputContext::update(Qt::InputMethodQueries queries)
{
    Q_UNUSED(queries)
    queryFocusObject();
}

void OpenKeyboardInputContext::queryFocusObject()
{
    if (!m_focusObject)
        return;
    const QVariant hintsVar = QInputMethod::queryFocusObject(Qt::ImHints, QVariant());
    if (hintsVar.isValid()) {
        m_hints = static_cast<Qt::InputMethodHints>(hintsVar.toInt());
        OpenKeyboardBridge::instance()->setInputMethodHints(int(m_hints));
    }
    const QVariant surrounding = QInputMethod::queryFocusObject(Qt::ImSurroundingText, QVariant());
    if (surrounding.isValid()) {
        const QString t = surrounding.toString();
        if (t != m_surroundingText) {
            m_surroundingText = t;
            OpenKeyboardBridge::instance()->setSurroundingText(t);
        }
    }
    const QVariant cursor = QInputMethod::queryFocusObject(Qt::ImCursorPosition, QVariant());
    if (cursor.isValid())
        m_cursorPosition = cursor.toInt();
    const QVariant anchor = QInputMethod::queryFocusObject(Qt::ImAnchorPosition, QVariant());
    if (anchor.isValid())
        m_anchorPosition = anchor.toInt();
}

void OpenKeyboardInputContext::reset() {}

void OpenKeyboardInputContext::commit() {}

void OpenKeyboardInputContext::invokeAction(QInputMethod::Action action, int cursorPosition)
{
    Q_UNUSED(cursorPosition)
    if (action == QInputMethod::Click) {
        // handled via focus change
    }
}

bool OpenKeyboardInputContext::filterEvent(const QEvent *event)
{
    Q_UNUSED(event)
    return false;
}

QRectF OpenKeyboardInputContext::keyboardRect() const { return m_keyboardRect; }
bool OpenKeyboardInputContext::isAnimating() const { return m_animating; }

void OpenKeyboardInputContext::showInputPanel()
{
    if (m_visible)
        return;
    m_visible = true;
    QRect screen = QRect(0, 0, 800, 480);
    if (QGuiApplication::primaryScreen())
        screen = QGuiApplication::primaryScreen()->geometry();
    // Fallback for offscreen where geometry is empty
    if (screen.width() <= 0 || screen.height() <= 0)
        screen = QRect(0, 0, 800, 480);
    m_keyboardRect = QRectF(0, screen.height() - 260, screen.width(), 260);
    OpenKeyboardBridge::instance()->setVisible(true);
    OpenKeyboardBridge::instance()->setKeyboardRect(m_keyboardRect);
    emitInputPanelVisibleChanged();
    emitKeyboardRectChanged();
    update(Qt::ImQueryAll);
}

void OpenKeyboardInputContext::hideInputPanel()
{
    if (!m_visible)
        return;
    m_visible = false;
    m_keyboardRect = QRectF();
    OpenKeyboardBridge::instance()->setVisible(false);
    OpenKeyboardBridge::instance()->setKeyboardRect(m_keyboardRect);
    emitInputPanelVisibleChanged();
    emitKeyboardRectChanged();
}

bool OpenKeyboardInputContext::isInputPanelVisible() const { return m_visible; }

QLocale OpenKeyboardInputContext::locale() const { return m_locale; }

Qt::LayoutDirection OpenKeyboardInputContext::inputDirection() const
{
    return m_locale.textDirection();
}

void OpenKeyboardInputContext::sendCommit(const QString &text, int replaceFrom, int replaceLength)
{
    if (!m_focusObject)
        return;
    QInputMethodEvent event;
    event.setCommitString(text, replaceFrom, replaceLength);
    QGuiApplication::sendEvent(m_focusObject, &event);
    queryFocusObject();
}

void OpenKeyboardInputContext::onCommitRequested(const QString &text)
{
    if (!m_focusObject)
        return;
    const QVariant selected = QInputMethod::queryFocusObject(Qt::ImCurrentSelection, QVariant());
    const QString sel = selected.isValid() ? selected.toString() : QString();
    if (!sel.isEmpty()) {
        // Replace selection
        const int selStart = qMin(m_cursorPosition, m_anchorPosition);
        const int selLen = qAbs(m_cursorPosition - m_anchorPosition);
        const int replaceFrom = selStart - m_cursorPosition;
        sendCommit(text, replaceFrom, selLen);
    } else {
        sendCommit(text, 0, 0);
    }
}

void OpenKeyboardInputContext::onDeleteRequested(int chars)
{
    if (!m_focusObject)
        return;
    const QVariant selected = QInputMethod::queryFocusObject(Qt::ImCurrentSelection, QVariant());
    const QString sel = selected.isValid() ? selected.toString() : QString();
    if (!sel.isEmpty()) {
        const int selStart = qMin(m_cursorPosition, m_anchorPosition);
        const int selLen = qAbs(m_cursorPosition - m_anchorPosition);
        const int replaceFrom = selStart - m_cursorPosition;
        sendCommit(QString(), replaceFrom, selLen);
    } else if (m_cursorPosition > 0) {
        sendCommit(QString(), -chars, chars);
    }
}

void OpenKeyboardInputContext::onHideRequested()
{
    hideInputPanel();
}
