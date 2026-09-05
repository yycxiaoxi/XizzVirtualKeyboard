// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
#include "virtualkeyboardinputcontext.h"
#include "bridge/virtualkeyboardbridge.h"
#include <QGuiApplication>
#include <QDebug>
#include <QMetaMethod>
#include <QMetaProperty>
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
    // feat-333: 插件版本构建戳。真机日志凭此一句可辨 .so 新旧(feat-332 第三道门、
    // feat-331 focusClearRequested 是否在运行包内), 不再靠告警反推。
    qInfo() << "[xizz] inputcontext build feat-333";
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
    if (!object)
        return false;
    const QMetaObject *mo = object->metaObject();
    if (mo->indexOfProperty("inputMethodHints") == -1)
        return false;
    // feat-329: 只读文本(如 SpinBox 不可编辑态)不算输入入口, 避免点到即弹键盘
    const int roIdx = mo->indexOfProperty("readOnly");
    if (roIdx != -1) {
        QVariant ro;
        const QMetaProperty prop = mo->property(roIdx);
        if (prop.isReadable())
            ro = object->property("readOnly");
        if (ro.isValid() && ro.toBool())
            return false;
    }
    // feat-332: 光有 inputMethodHints 属性不够, 还得真能响应 inputMethodQuery 元调用。
    // QQuickSpinBox/QQuickComboBox 本体带 hints 属性(透传给内部编辑器用), 但头文件里
    // 无 Q_INVOKABLE inputMethodQuery, QInputMethod::queryFocusObject 经 invokeMethod
    // 查它必报 "No such method ...::inputMethodQuery" 且拿不到值。按方法名(而非全签名)
    // 遍历元对象(含继承链, TextField 靠继承 QQuickTextInput 命中), 找不到判非文本。
    for (const QMetaObject *m = mo; m; m = m->superClass()) {
        for (int i = 0; i < m->methodCount(); ++i) {
            if (m->method(i).name() == QByteArrayLiteral("inputMethodQuery"))
                return true;
        }
    }
    return false;
}

void XizzVirtualKeyboardInputContext::setFocusObject(QObject *object)
{
    if (object && isKeyboardChrome(object))
        return;
    if (object && !isTextInput(object)) {
        if (m_visible)
            hideInputPanel();
        else
            clearFocusState();
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
    // QInputMethod::queryFocusObject() meta-calls "inputMethodQuery" on the
    // *window's current* focus object (QGuiApplication::focusObject()), which
    // during focus transitions / popup teardown can be a plain QQuickItem or a
    // FocusScope wrapper instead of a real text input. Those objects expose no
    // invokable inputMethodQuery, so each call makes Qt log
    // "No such method ...::inputMethodQuery(Qt::InputMethodQuery,QVariant)".
    // Only query while the window focus really is a text-capable object.
    QObject *windowFocus = QGuiApplication::focusObject();
    if (!windowFocus || !isTextInput(windowFocus))
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
    clearFocusState();
    // feat-331: 真收起才到此处(幂等守卫上), 通知宿主可顺手清旧输入焦点。
    // 库只发信号, 真正清焦点由宿主 QML 经 callLater 做(防派发栈重入/抢新焦点)。
    XizzVirtualKeyboardBridge::instance()->requestFocusClear();
    emitInputPanelVisibleChanged();
    emitKeyboardRectChanged();
}

void XizzVirtualKeyboardInputContext::clearFocusState()
{
    // feat-329: 焦点离开文本输入时清残留, 避免密码掩码圆点残留在预览条下次闪现。幂等。
    m_surroundingText.clear();
    m_hints = Qt::ImhNone;
    m_cursorPosition = 0;
    m_anchorPosition = 0;
    auto *bridge = XizzVirtualKeyboardBridge::instance();
    bridge->setSurroundingText(QString());
    // setInputMethodHints(0) 连带 isPassword=false, 预览条掩码一并解除
    bridge->setInputMethodHints(int(Qt::ImhNone));
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
    if (m_submitting)
        return;
    if (!m_focusObject) {
        hideInputPanel();
        return;
    }
    m_submitting = true;
    // Snapshot the target. The KeyPress below runs QML synchronously inside
    // sendEvent(): a single-line TextInput reacts to Return with accepted() and
    // editingFinished(), and XuiLoginDialog-style handlers use
    // "onEditingFinished: focus = false", so the focused item can drop focus
    // (Qt moves the window focus to a FocusScope/Item) or even be destroyed
    // while we are still inside the press dispatch. setFocusObject() clears
    // m_focusObject at that point. Sending KeyRelease to a focus object that no
    // longer exists / no longer owns the key would crash — re-check before it.
    const QPointer<QObject> focus = m_focusObject;
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, QStringLiteral("\n"));
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier, QStringLiteral("\n"));
    QGuiApplication::sendEvent(focus, &press);
    if (m_focusObject && m_focusObject == focus)
        QGuiApplication::sendEvent(focus, &release);
    m_submitting = false;
    hideInputPanel();
}
