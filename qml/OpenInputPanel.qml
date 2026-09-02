// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
import QtQuick 2.15

Item {
    id: root
    focus: false
    property bool active: false
    property string actionLabel: "\u641c\u7d22"
    property color themeColor: "#00C7A0"
    property bool showCandidateBar: true
    signal actionTriggered(string text)
    signal closed()

    width: parent ? parent.width : 800
    height: keyboardView.implicitHeight
    y: parent ? (active ? parent.height - height : parent.height) : 0
    visible: active
    z: 9999

    Behavior on y { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }

    Timer { id: primeTimer; interval: 60; repeat: false; onTriggered: syncBufferFromFocus() }

    function syncBufferFromFocus(t) {
        if (keyboardView && keyboardView.engine) {
            var v = t !== undefined ? t : (Qt.inputMethod && typeof Qt.inputMethod.surroundingText !== "undefined" ? Qt.inputMethod.surroundingText || "" : "")
            if (keyboardView.engine.buffer !== v)
                keyboardView.engine.buffer = v
        }
    }

    property bool _primed: false

    function hide() {
        if (openKeyboardBridge) openKeyboardBridge.hideKeyboard()
        else { active = false; root.closed() }
    }

    Connections {
        target: openKeyboardBridge
        function onVisibleChanged() {
            if (openKeyboardBridge) {
                root.active = openKeyboardBridge.visible
                if (openKeyboardBridge.visible) {
                    var hints = (typeof openKeyboardBridge.inputMethodHints !== "undefined" ? openKeyboardBridge.inputMethodHints : 0)
                    if (!hints && typeof Qt.inputMethod !== "undefined") hints = (Qt.inputMethod.inputMethodHints || 0)
                    keyboardView.applyHints(hints)
                    if (!root._primed) {
                        root._primed = true
                        // Let the input method settle after first show, then sync
                        primeTimer.start()
                    }
                    syncBufferFromFocus()
                } else if (!openKeyboardBridge.visible) {
                    root._primed = false
                }
            }
        }
    }

    Connections {
        target: openKeyboardBridge
        function onSurroundingChanged(text) { syncBufferFromFocus(text) }
        function onIsPasswordChanged() {
            if (openKeyboardBridge && keyboardView && keyboardView.applyHints)
                keyboardView.applyHints(openKeyboardBridge.inputMethodHints || (openKeyboardBridge.isPassword ? 0x8 : 0))
        }
        function onInputMethodHintsChanged() {
            if (openKeyboardBridge && keyboardView && keyboardView.applyHints)
                keyboardView.applyHints(openKeyboardBridge.inputMethodHints || 0)
        }
    }

    Connections {
        target: Qt.inputMethod
        ignoreUnknownSignals: true
        function onVisibleChanged() {
            if (!openKeyboardBridge || !openKeyboardBridge.visible)
                root.active = Qt.inputMethod.visible
        }
    }

    OpenKeyboard {
        id: keyboardView
        width: root.width
        actionLabel: root.actionLabel
        actionColor: root.themeColor
        showCandidateBar: root.showCandidateBar
        bridge: openKeyboardBridge
        onActionTriggered: {
            var t = ""
            if (typeof Qt.inputMethod !== "undefined" && Qt.inputMethod.surroundingText !== undefined)
                t = Qt.inputMethod.surroundingText
            root.actionTriggered(t)
            root.hide()
        }
    }

    Component.onCompleted: {
        if (openKeyboardBridge) root.active = openKeyboardBridge.visible
        if (typeof Qt.inputMethod !== "undefined" && Qt.inputMethod.visible) root.active = true
    }
}
