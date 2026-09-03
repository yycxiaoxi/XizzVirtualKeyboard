// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
#pragma once
#include <qpa/qplatforminputcontext.h>
#include <QPointer>
#include <QRectF>
#include <QLocale>

class XizzVirtualKeyboardInputContext : public QPlatformInputContext
{
    Q_OBJECT
public:
    explicit XizzVirtualKeyboardInputContext(QObject *parent = nullptr);
    ~XizzVirtualKeyboardInputContext() override;

    bool isValid() const override;
    void reset() override;
    void commit() override;
    void update(Qt::InputMethodQueries queries) override;
    void invokeAction(QInputMethod::Action action, int cursorPosition) override;
    bool filterEvent(const QEvent *event) override;
    QRectF keyboardRect() const override;
    bool isAnimating() const override;
    void showInputPanel() override;
    void hideInputPanel() override;
    bool isInputPanelVisible() const override;
    QLocale locale() const override;
    Qt::LayoutDirection inputDirection() const override;
    void setFocusObject(QObject *object) override;

private slots:
    void onSubmitRequested();
    void onCommitRequested(const QString &text);
    void onDeleteRequested(int chars);
    void onHideRequested();

private:
    void sendCommit(const QString &text, int replaceFrom = 0, int replaceLength = 0);
    void queryFocusObject();
    void clearFocusState();

    QPointer<QObject> m_focusObject;
    bool m_visible = false;
    bool m_animating = false;
    bool m_submitting = false;
    QRectF m_keyboardRect;
    QLocale m_locale;
    Qt::InputMethodHints m_hints = Qt::ImhNone;
    QString m_surroundingText;
    int m_cursorPosition = 0;
    int m_anchorPosition = 0;
};
