// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
#include "virtualkeyboardplugin.h"
#include "virtualkeyboardinputcontext.h"

XizzVirtualKeyboardPlugin::XizzVirtualKeyboardPlugin(QObject *parent)
    : QPlatformInputContextPlugin(parent) {}

QPlatformInputContext *XizzVirtualKeyboardPlugin::create(const QString &key, const QStringList &paramList)
{
    Q_UNUSED(paramList)
    if (key.compare(QStringLiteral("xizzvirtualkeyboard"), Qt::CaseInsensitive) == 0)
        return new XizzVirtualKeyboardInputContext;
    return nullptr;
}
