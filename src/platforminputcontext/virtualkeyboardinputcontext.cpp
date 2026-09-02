// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
#include "virtualkeyboardinputcontext.h"
#include "bridge/virtualkeyboardbridge.h"
#include <QGuiApplication>
#include <QScreen>
#include <QInputMethodEvent>
#include <QInputMethod>

XizzVirtualKeyboardInputContext::XizzVirtualKeyboardInputContext(QObject *parent)
    : QPlatformInputContext()
{
    Q_UNUSED(parent)
    auto *bridge = XizzVirtualKeyboardBridge::instance();
    connect(bridge, &XizzVirtualKeyboardBridge::commitRequested,
            this, &XizzVirtualKeyboardInputContext::onCommitRequested);
    connect(bridge, &XizzVirtualKeyboardBridge::deleteRequested,
            this, &XizzVirtualKeyboardInputContext::onDeleteRequested);
    connect(bridge, &XizzVirtualKeyboardBridge::hideRequested,
            this, &XizzVirtualKeyboardInputContext::onHideRequested);
    connect(bridge, &XizzVirtualKeyboardBridge::submitRequested,
            this, &XizzVirtualKeyboardInputContext::onSubmitRequested);
}

XizzVirtualKeyboardInputContext::~XizzVirtualKeyboardInputContext() = default;

bool XizzVirtualKeyboardInputContext::isValid() const { return true; }

static bool isKeyboardChrome(QObject *object)
{
    for (QObject *cur = object; cur; cur = cur->parent()) {
        const QMetaObject *mo = cur->metaObject();
        const QByteArray cn = mo->className();
        if (cn.contains("KeyButton") || cn.contains("TextKey") || cn.contains("FunctionKey")
            || cn.contains("ActionKey") || cn.contains("CandidateBar")
            || cn.contains("XizzVirtualKeyboard") || cn.contains("OpenInputPanel") || cn.contains("ToolButton"))
            return true;
        if (mo->indexOfProperty("keyText") != -1 || mo->indexOfProperty("keyChar") != -1)
            return true;
        // XizzVirtualKeyboard / OpenInputPanel roots are plain QQuickRectangle/QQuickItem but expose unique properties
        if (mo->indexOfProperty("showCandidateBar") != -1)
            return true;
    }
    return false;
}

static bool isTextInput(QObject *object)
{
    return object && object->metaObject()->indexOfProperty("inputMethodHints") != -1;
}

void XizzVirtualKeyboardInputContext::setFocusObject(QObject *object)
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

void XizzVirtualKeyboardInputContext::update(Qt::InputMethodQueries queries)
{
    Q_UNUSED(queries)
    queryFocusObject();
}

void XizzVirtualKeyboardInputContext::queryFocusObject()
{
    if (!m_focusObject)
        return;
    const QVariant hintsVar = QInputMethod::queryFocusObject(Qt::ImHints, QVariant());
    if (hintsVar.isValid()) {
        m_hints = static_cast<Qt::InputMethodHints>(hintsVar.toInt());
        XizzVirtualKeyboardBridge::instance()->setInputMethodHints(int(m_hints));
    }
    const QVariant surrounding = QInputMethod::queryFocusObject(Qt::ImSurroundingText, QVariant());
    if (surrounding.isValid()) {
        const QString t = surrounding.toString();
        if (t != m_surroundingText) {
            m_surroundingText = t;
            XizzVirtualKeyboardBridge::instance()->setSurroundingText(t);
        }
    }
    const QVariant cursor = QInputMethod::queryFocusObject(Qt::ImCursorPosition, QVariant());
    if (cursor.isValid())
        m_cursorPosition = cursor.toInt();
    const QVariant anchor = QInputMethod::queryFocusObject(Qt::ImAnchorPosition, QVariant());
    if (anchor.isValid())
        m_anchorPosition = anchor.toInt();
}

void XizzVirtualKeyboardInputContext::reset() {}

void XizzVirtualKeyboardInputContext::commit() {}

void XizzVirtualKeyboardInputContext::invokeAction(QInputMethod::Action action, int cursorPosition)
{
    Q_UNUSED(cursorPosition)
    if (action == QInputMethod::Click) {
        // handled via focus change
    }
}

bool XizzVirtualKeyboardInputContext::filterEvent(const QEvent *event)
{
    Q_UNUSED(event)
    return false;
}

QRectF XizzVirtualKeyboardInputContext::keyboardRect() const { return m_keyboardRect; }
bool XizzVirtualKeyboardInputContext::isAnimating() const { return m_animating; }

void XizzVirtualKeyboardInputContext::showInputPanel()
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
    XizzVirtualKeyboardBridge::instance()->setVisible(true);
    XizzVirtualKeyboardBridge::instance()->setKeyboardRect(m_keyboardRect);
    emitInputPanelVisibleChanged();
    emitKeyboardRectChanged();
    update(Qt::ImQueryAll);
}

void XizzVirtualKeyboardInputContext::hideInputPanel()
{
    if (!m_visible)
        return;
    m_visible = false;
    m_keyboardRect = QRectF();
    XizzVirtualKeyboardBridge::instance()->setVisible(false);
    XizzVirtualKeyboardBridge::instance()->setKeyboardRect(m_keyboardRect);
    emitInputPanelVisibleChanged();
    emitKeyboardRectChanged();
}

bool XizzVirtualKeyboardInputContext::isInputPanelVisible() const { return m_visible; }

QLocale XizzVirtualKeyboardInputContext::locale() const { return m_locale; }

Qt::LayoutDirection XizzVirtualKeyboardInputContext::inputDirection() const
{
    return m_locale.textDirection();
}

void XizzVirtualKeyboardInputContext::sendCommit(const QString &text, int replaceFrom, int replaceLength)
{
    if (!m_focusObject)
        return;
    QInputMethodEvent event;
    event.setCommitString(text, replaceFrom, replaceLength);
    QGuiApplication::sendEvent(m_focusObject, &event);
    queryFocusObject();
}

void XizzVirtualKeyboardInputContext::onCommitRequested(const QString &text)
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

void XizzVirtualKeyboardInputContext::onDeleteRequested(int chars)
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

void XizzVirtualKeyboardInputContext::onHideRequested()
{
    hideInputPanel();
}

void XizzVirtualKeyboardInputContext::onSubmitRequested()
{
    if (!m_focusObject)
        return;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, QStringLiteral("\n"));
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier, QStringLiteral("\n"));
    QGuiApplication::sendEvent(m_focusObject, &press);
    QGuiApplication::sendEvent(m_focusObject, &release);
    hideInputPanel();
}
