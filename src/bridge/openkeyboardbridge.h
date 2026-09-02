// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
#pragma once
#include <QObject>
#include <QRectF>
#include <QLocale>

#if __has_include("openkeyboard_export.h")
#include "openkeyboard_export.h"
#elif __has_include("OpenKeyboard/openkeyboard_export.h")
#include "OpenKeyboard/openkeyboard_export.h"
#else
#ifndef OPENKEYBOARD_EXPORT
#define OPENKEYBOARD_EXPORT
#endif
#endif

class OPENKEYBOARD_EXPORT OpenKeyboardBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged)
    Q_PROPERTY(QRectF keyboardRect READ keyboardRect NOTIFY keyboardRectChanged)
    Q_PROPERTY(QString styleName READ styleName NOTIFY styleNameChanged)
    Q_PROPERTY(QLocale locale READ locale NOTIFY localeChanged)
    Q_PROPERTY(QString surroundingText READ surroundingText NOTIFY surroundingChanged)
    Q_PROPERTY(bool isPassword READ isPassword NOTIFY isPasswordChanged)
    Q_PROPERTY(int inputMethodHints READ inputMethodHints NOTIFY inputMethodHintsChanged)

public:
    static OpenKeyboardBridge *instance();

    bool visible() const { return m_visible; }
    QRectF keyboardRect() const { return m_keyboardRect; }
    QString styleName() const { return m_styleName; }
    QLocale locale() const { return m_locale; }

    void setVisible(bool v);
    void setKeyboardRect(const QRectF &r);
    void setStyleName(const QString &name);
    void setLocale(const QLocale &locale);

    Q_INVOKABLE void commitText(const QString &text);
    Q_INVOKABLE void deleteSurrounding(int chars);
    Q_INVOKABLE void hideKeyboard();
    Q_INVOKABLE void setSurroundingText(const QString &text);
    QString surroundingText() const { return m_surroundingText; }
    bool isPassword() const { return m_isPassword; }
    int inputMethodHints() const { return m_inputMethodHints; }
    void setIsPassword(bool v);
    void setInputMethodHints(int hints);

signals:
    void visibleChanged();
    void keyboardRectChanged();
    void styleNameChanged();
    void localeChanged();
    void commitRequested(const QString &text);
    void deleteRequested(int chars);
    void hideRequested();
    void surroundingChanged(const QString &text);
    void isPasswordChanged();
    void inputMethodHintsChanged();

private:
    explicit OpenKeyboardBridge(QObject *parent = nullptr);
    bool m_visible = false;
    QRectF m_keyboardRect;
    QString m_styleName = QStringLiteral("compact");
    QLocale m_locale;
    QString m_surroundingText;
    bool m_isPassword = false;
    int m_inputMethodHints = 0;
};
