// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
#include "openkeyboardbridge.h"
#include <QByteArray>
#include <QGuiApplication>
#include <QInputMethod>

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
