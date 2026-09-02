// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
import QtQuick 2.15
import XizzVirtualKeyboard.Internal 1.0

Item {
    id: root
    focus: false
    // Resolved from the XizzVirtualKeyboard.Internal singleton registered by the
    // plugin itself; null (and graceful degradation) if registration is absent.
    property var bridge: (typeof XizzVirtualKeyboardBridge !== "undefined") ? XizzVirtualKeyboardBridge : null
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
        if (bridge) bridge.hideKeyboard()
        else { active = false; root.closed() }
    }

    Connections {
        target: bridge
        function onVisibleChanged() {
            if (bridge) {
                root.active = bridge.visible
                if (bridge.visible) {
                    var hints = (typeof bridge.inputMethodHints !== "undefined" ? bridge.inputMethodHints : 0)
                    if (!hints && typeof Qt.inputMethod !== "undefined") hints = (Qt.inputMethod.inputMethodHints || 0)
                    keyboardView.applyHints(hints)
                    if (!root._primed) {
                        root._primed = true
                        // Let the input method settle after first show, then sync
                        primeTimer.start()
                    }
                    syncBufferFromFocus()
                } else {
                    root._primed = false
                }
            }
        }
    }

    Connections {
        target: bridge
        function onSurroundingChanged(text) { syncBufferFromFocus(text) }
        function onIsPasswordChanged() {
            if (bridge && keyboardView && keyboardView.applyHints)
                keyboardView.applyHints(bridge.inputMethodHints || (bridge.isPassword ? 0x8 : 0))
        }
        function onInputMethodHintsChanged() {
            if (bridge && keyboardView && keyboardView.applyHints)
                keyboardView.applyHints(bridge.inputMethodHints || 0)
        }
    }

    Connections {
        target: Qt.inputMethod
        ignoreUnknownSignals: true
        function onVisibleChanged() {
            if (!bridge || !bridge.visible)
                root.active = Qt.inputMethod.visible
        }
    }

    XizzVirtualKeyboard {
        id: keyboardView
        width: root.width
        actionLabel: root.actionLabel
        actionColor: root.themeColor
        showCandidateBar: root.showCandidateBar
        bridge: root.bridge
        onActionTriggered: {
            var t = ""
            if (typeof Qt.inputMethod !== "undefined" && Qt.inputMethod.surroundingText !== undefined)
                t = Qt.inputMethod.surroundingText
            root.actionTriggered(t)
            root.hide()
        }
    }

    Component.onCompleted: {
        if (bridge) root.active = bridge.visible
        if (typeof Qt.inputMethod !== "undefined" && Qt.inputMethod.visible) root.active = true
    }
}
