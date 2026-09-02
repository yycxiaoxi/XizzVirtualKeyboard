// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    // The only host-side integration line: pick OpenKeyboard as the platform
    // input method. The plugin (and the QML module it carries) is discovered
    // by Qt itself from <appdir>/platforminputcontexts — no setup calls, no
    // context properties, nothing to link.
    qputenv("QT_IM_MODULE", QByteArray("openkeyboard"));
    if (qgetenv("QT_QUICK_BACKEND").isEmpty())
        qputenv("QT_QUICK_BACKEND", QByteArray("software"));

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return 1;
    return app.exec();
}
