// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
#pragma once
#include <qpa/qplatforminputcontextplugin_p.h>

class XizzVirtualKeyboardPlugin : public QPlatformInputContextPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformInputContextFactoryInterface_iid FILE "platforminputcontext.json")
public:
    explicit XizzVirtualKeyboardPlugin(QObject *parent = nullptr);
    QPlatformInputContext *create(const QString &key, const QStringList &paramList) override;
};
