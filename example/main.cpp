// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "bridge/openkeyboardbridge.h"

int main(int argc, char *argv[])
{
    // Ensure plugin is discoverable when running from build dir (QT_PLUGIN_PATH)
    // After install, QT_PLUGIN_PATH is set via environment or qt.conf
    qputenv("QT_IM_MODULE", QByteArray("openkeyboard"));
    // Use software backend on embedded/linuxfb and headless
    if (qgetenv("QT_QUICK_BACKEND").isEmpty())
        qputenv("QT_QUICK_BACKEND", QByteArray("software"));
    // Style selection via OPENKEYBOARD_STYLE (compact | default), default is compact
    // qputenv("OPENKEYBOARD_STYLE", QByteArray("compact"));
    // Register build-tree plugin path BEFORE QGuiApplication so the
    // platforminputcontext factory already sees it during construction.
    // The path is injected by CMake (OPENKEYBOARD_PLUGINS_DIR) so it works
    // on any machine / shadow build; the applicationDirPath fallbacks cover
    // installs and custom layouts.
#ifdef OPENKEYBOARD_PLUGINS_DIR
    QCoreApplication::addLibraryPath(QStringLiteral(OPENKEYBOARD_PLUGINS_DIR));
#endif
    QGuiApplication app(argc, argv);
    app.addLibraryPath(app.applicationDirPath() + QStringLiteral("/../plugins"));
    app.addLibraryPath(app.applicationDirPath() + QStringLiteral("/../../plugins"));
    QQmlApplicationEngine engine;
#ifdef OPENKEYBOARD_SOURCE_ROOT
    engine.addImportPath(QStringLiteral(OPENKEYBOARD_SOURCE_ROOT));
#endif
    engine.addImportPath(QStringLiteral("../"));
    engine.addImportPath(QStringLiteral("../../"));
    engine.rootContext()->setContextProperty(QStringLiteral("openKeyboardBridge"), OpenKeyboardBridge::instance());
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;
    return app.exec();
}
