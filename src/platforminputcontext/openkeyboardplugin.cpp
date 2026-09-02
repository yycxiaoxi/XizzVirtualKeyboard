// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
#include "openkeyboardplugin.h"
#include "openkeyboardinputcontext.h"

OpenKeyboardPlugin::OpenKeyboardPlugin(QObject *parent)
    : QPlatformInputContextPlugin(parent) {}

QPlatformInputContext *OpenKeyboardPlugin::create(const QString &key, const QStringList &paramList)
{
    Q_UNUSED(paramList)
    if (key.compare(QStringLiteral("openkeyboard"), Qt::CaseInsensitive) == 0)
        return new OpenKeyboardInputContext;
    return nullptr;
}
