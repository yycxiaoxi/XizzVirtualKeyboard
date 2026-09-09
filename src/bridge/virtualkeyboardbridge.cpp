// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
#include "virtualkeyboardbridge.h"
#include <QByteArray>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QInputMethod>
#include <QQmlEngine>

// Wiring without host code: this whole library lives inside the platform
// input context plugin, which Qt loads by itself when QT_IM_MODULE=xizzvirtualkeyboard.
// The static initializer below runs at that dlopen — long before any engine —
// and does two things:
//   (a) registers instance() as the QML singleton "xizzVirtualKeyboardBridge" in
//       module XizzVirtualKeyboard.Internal, so panel QML resolves it without a
//       host-side setContextProperty;
//   (b) injects the source root into QML2_IMPORT_PATH/QML_IMPORT_PATH so
//       "import XizzVirtualKeyboard 1.0" resolves from the filesystem qmldir in
//       build-tree runs (Qt5). A "qrc:" URL cannot travel through those env
//       vars (colon collides with the list separator); Qt5 packaged hosts
//       install the module into Qt's qml dir instead. Qt6 needs nothing
//       here: qt_add_qml_module registers the module at plugin load and
//       qrc:/qt/qml is a default import path.
struct XizzVirtualKeyboardAutoSetup {
    XizzVirtualKeyboardAutoSetup() {
        XizzVirtualKeyboardBridge::registerQmlSingleton();
#ifdef XIZZVIRTUALKEYBOARD_SOURCE_ROOT
        const QByteArray fs = QByteArrayLiteral(XIZZVIRTUALKEYBOARD_SOURCE_ROOT);
        if (!fs.isEmpty()) {
            auto inject = [](const char *var, const QByteArray &addition) {
                const QByteArray existing = qgetenv(var);
                if (existing.isEmpty()) {
                    qputenv(var, addition);
                } else if (!existing.split(':').contains(addition)) {
                    qputenv(var, existing + ':' + addition);
                }
            };
            inject("QML2_IMPORT_PATH", fs);
            inject("QML_IMPORT_PATH", fs);
        }
#endif
    }
};
static XizzVirtualKeyboardAutoSetup s_xizzVirtualKeyboardAutoSetup;

void XizzVirtualKeyboardBridge::registerQmlSingleton()
{
    static bool registered = false;
    if (registered)
        return;
    registered = true;
    // Module "XizzVirtualKeyboard.Internal" is registered purely programmatically —
    // no qmldir file — so panel-internal QML imports it without colliding
    // with the public XizzVirtualKeyboard module. QML type names must start
    // uppercase, hence "XizzVirtualKeyboardBridge" (not the camelCase property
    // style used by the old host-side context property). The instance is
    // parented to QCoreApplication by instance() (lazily, also before the
    // app exists).
    qmlRegisterSingletonInstance("XizzVirtualKeyboard.Internal", 1, 0,
                                 "XizzVirtualKeyboardBridge", instance());
}

static XizzVirtualKeyboardBridge *s_instance = nullptr;

XizzVirtualKeyboardBridge *XizzVirtualKeyboardBridge::instance()
{
    if (!s_instance) {
        s_instance = new XizzVirtualKeyboardBridge(qApp);
    } else if (s_instance->parent() == nullptr && qApp) {
        s_instance->setParent(qApp);
    }
    return s_instance;
}

XizzVirtualKeyboardBridge::XizzVirtualKeyboardBridge(QObject *parent)
    : QObject(parent)
{
    const QByteArray env = qgetenv("XIZZVIRTUALKEYBOARD_STYLE");
    if (!env.isEmpty())
        m_styleName = QString::fromUtf8(env);
}

void XizzVirtualKeyboardBridge::setVisible(bool v)
{
    if (m_visible == v)
        return;
    m_visible = v;
    emit visibleChanged();
}

void XizzVirtualKeyboardBridge::setKeyboardRect(const QRectF &r)
{
    if (m_keyboardRect == r)
        return;
    m_keyboardRect = r;
    emit keyboardRectChanged();
}

void XizzVirtualKeyboardBridge::setStyleName(const QString &name)
{
    if (m_styleName == name)
        return;
    m_styleName = name;
    emit styleNameChanged();
}

void XizzVirtualKeyboardBridge::setLocale(const QLocale &locale)
{
    if (m_locale == locale)
        return;
    m_locale = locale;
    emit localeChanged();
}

void XizzVirtualKeyboardBridge::commitText(const QString &text)
{
    if (text.isEmpty())
        return;
    emit commitRequested(text);
}

void XizzVirtualKeyboardBridge::deleteSurrounding(int chars)
{
    emit deleteRequested(chars);
}

void XizzVirtualKeyboardBridge::clearAll()
{
    emit clearAllRequested();
}

void XizzVirtualKeyboardBridge::hideKeyboard()
{
    emit hideRequested();
}

void XizzVirtualKeyboardBridge::requestFocusClear()
{
    emit focusClearRequested();
}

void XizzVirtualKeyboardBridge::submit()
{
    emit submitRequested();
}

void XizzVirtualKeyboardBridge::setSurroundingText(const QString &text)
{
    if (m_surroundingText == text)
        return;
    m_surroundingText = text;
    emit surroundingChanged(text);
}

void XizzVirtualKeyboardBridge::setIsPassword(bool v)
{
    if (m_isPassword == v)
        return;
    m_isPassword = v;
    emit isPasswordChanged();
}

void XizzVirtualKeyboardBridge::setInputMethodHints(int hints)
{
    if (m_inputMethodHints == hints)
        return;
    m_inputMethodHints = hints;
    emit inputMethodHintsChanged();
    setIsPassword(hints & int(Qt::ImhHiddenText));
}
