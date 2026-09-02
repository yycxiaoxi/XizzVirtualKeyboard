// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
#include "openkeyboardbridge.h"
#include <QByteArray>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QInputMethod>
#include <QQmlEngine>

// Wiring without host code: this whole library lives inside the platform
// input context plugin, which Qt loads by itself when QT_IM_MODULE=openkeyboard.
// The static initializer below runs at that dlopen — long before any engine —
// and does two things:
//   (a) registers instance() as the QML singleton "openKeyboardBridge" in
//       module OpenKeyboard.Internal, so panel QML resolves it without a
//       host-side setContextProperty;
//   (b) injects the source root into QML2_IMPORT_PATH/QML_IMPORT_PATH so
//       "import OpenKeyboard 1.0" resolves from the filesystem qmldir in
//       build-tree runs (Qt5). A "qrc:" URL cannot travel through those env
//       vars (colon collides with the list separator); Qt5 packaged hosts
//       install the module into Qt's qml dir instead. Qt6 needs nothing
//       here: qt_add_qml_module registers the module at plugin load and
//       qrc:/qt/qml is a default import path.
struct OpenKeyboardAutoSetup {
    OpenKeyboardAutoSetup() {
        OpenKeyboardBridge::registerQmlSingleton();
#ifdef OPENKEYBOARD_SOURCE_ROOT
        const QByteArray fs = QByteArrayLiteral(OPENKEYBOARD_SOURCE_ROOT);
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
static OpenKeyboardAutoSetup s_openKeyboardAutoSetup;

void OpenKeyboardBridge::registerQmlSingleton()
{
    static bool registered = false;
    if (registered)
        return;
    registered = true;
    // Module "OpenKeyboard.Internal" is registered purely programmatically —
    // no qmldir file — so panel-internal QML imports it without colliding
    // with the public OpenKeyboard module. QML type names must start
    // uppercase, hence "OpenKeyboardBridge" (not the camelCase property
    // style used by the old host-side context property). The instance is
    // parented to QCoreApplication by instance() (lazily, also before the
    // app exists).
    qmlRegisterSingletonInstance("OpenKeyboard.Internal", 1, 0,
                                 "OpenKeyboardBridge", instance());
}

static OpenKeyboardBridge *s_instance = nullptr;

OpenKeyboardBridge *OpenKeyboardBridge::instance()
{
    if (!s_instance) {
        s_instance = new OpenKeyboardBridge(qApp);
    } else if (s_instance->parent() == nullptr && qApp) {
        s_instance->setParent(qApp);
    }
    return s_instance;
}

OpenKeyboardBridge::OpenKeyboardBridge(QObject *parent)
    : QObject(parent)
{
    const QByteArray env = qgetenv("OPENKEYBOARD_STYLE");
    if (!env.isEmpty())
        m_styleName = QString::fromUtf8(env);
}

void OpenKeyboardBridge::setVisible(bool v)
{
    if (m_visible == v)
        return;
    m_visible = v;
    emit visibleChanged();
}

void OpenKeyboardBridge::setKeyboardRect(const QRectF &r)
{
    if (m_keyboardRect == r)
        return;
    m_keyboardRect = r;
    emit keyboardRectChanged();
}

void OpenKeyboardBridge::setStyleName(const QString &name)
{
    if (m_styleName == name)
        return;
    m_styleName = name;
    emit styleNameChanged();
}

void OpenKeyboardBridge::setLocale(const QLocale &locale)
{
    if (m_locale == locale)
        return;
    m_locale = locale;
    emit localeChanged();
}

void OpenKeyboardBridge::commitText(const QString &text)
{
    if (text.isEmpty())
        return;
    emit commitRequested(text);
}

void OpenKeyboardBridge::deleteSurrounding(int chars)
{
    emit deleteRequested(chars);
}

void OpenKeyboardBridge::hideKeyboard()
{
    emit hideRequested();
}

void OpenKeyboardBridge::setSurroundingText(const QString &text)
{
    if (m_surroundingText == text)
        return;
    m_surroundingText = text;
    emit surroundingChanged(text);
}

void OpenKeyboardBridge::setIsPassword(bool v)
{
    if (m_isPassword == v)
        return;
    m_isPassword = v;
    emit isPasswordChanged();
}

void OpenKeyboardBridge::setInputMethodHints(int hints)
{
    if (m_inputMethodHints == hints)
        return;
    m_inputMethodHints = hints;
    emit inputMethodHintsChanged();
    setIsPassword(hints & int(Qt::ImhHiddenText));
}
