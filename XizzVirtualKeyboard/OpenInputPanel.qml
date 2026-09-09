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
    property color themeColor: "#00C7A0"
    property bool showCandidateBar: true
    signal closed()
    // feat-331: 键盘真收起时 bridge.focusClearRequested 的面板级转发。
    // 宿主订阅此信号清旧输入焦点, 不直连 Internal 单例。
    signal focusClearRequested()

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
                    // feat-329: 面板收起只清预览, 不删真实文本。
                    // C++ 侧 hideInputPanel 已清 bridge.surroundingText, 此处再清
                    // engine.buffer 本地镜像(不走 deleteSurrounding), 避免旧圆点下次闪现。
                    if (keyboardView && keyboardView.engine)
                        keyboardView.engine.clearPreview()
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
        // feat-331: 转发键盘真收起意图, 宿主订阅后清旧输入焦点
        function onFocusClearRequested() { root.focusClearRequested() }
    }

    // feat-329: Qt.inputMethod.visible 兜底只在 bridge 缺失时生效。
    // bridge 存在时它是唯一真相源: bridge=false 即强制收起, 避免 Qt 侧
    // visible 落后把已 hide 的面板重新点亮(如下拉/comboBox 场景的"复活")。
    Connections {
        target: Qt.inputMethod
        ignoreUnknownSignals: true
        function onVisibleChanged() {
            if (!bridge) {
                root.active = Qt.inputMethod.visible
            } else if (!bridge.visible) {
                root.active = false
            }
        }
    }

    // 面板按压地板：键帽之外的空隙（键距/预览条空白/内边距）原先会把按压
    // 透传到面板下层的控件与页面（点 a 键附近落到键盘外即触发底下菜单跳页）。
    // 声明在 keyboardView 之前 = 垫底：键帽/按钮在上层照常消费，落到空隙的
    // 按压在此终止，不再穿出键盘面板。
    MouseArea {
        anchors.fill: parent
    }

    XizzVirtualKeyboard {
        id: keyboardView
        width: root.width
        showCandidateBar: root.showCandidateBar
        bridge: root.bridge
    }

    Component.onCompleted: {
        if (bridge) root.active = bridge.visible
        // feat-329: bridge 存在时不以 Qt.inputMethod.visible 点亮(见上), 避免启动即误弹
    }
}
