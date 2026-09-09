// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
import QtQuick 2.15

QtObject {
    id: engine
    property var bridge: null
    property bool upperCase: false
    property bool capsLock: false
    property int page: 0
    property var candidates: ["\u5927\u5bb6\u90fd\u5728\u505a", "\u7f8e\u5473\u4e0a\u65b0"]
    property string buffer: ""
    property int _lastShiftMs: 0

    function toggleShift() {
        // Single tap latches uppercase (fixed); tap again to release.
        upperCase = !upperCase
    }

    function formatKey(ch) {
        if (ch.length === 1 && ch >= "a" && ch <= "z" && upperCase) return ch.toUpperCase()
        if (ch === "&&") return "&"
        return ch
    }

    function commit(text) {
        var t = formatKey(text)
        buffer += t
        if (bridge && bridge.commitText) bridge.commitText(t)
        else console.warn("XizzVirtualKeyboard: bridge not connected")
    }

    function commitRaw(text) {
        buffer += text
        if (bridge && bridge.commitText) bridge.commitText(text)
        else console.warn("XizzVirtualKeyboard: bridge not connected")
    }

    function backspace() {
        if (buffer.length > 0) buffer = buffer.slice(0, buffer.length - 1)
        if (bridge && bridge.deleteSurrounding) bridge.deleteSurrounding(1)
        else console.warn("XizzVirtualKeyboard: bridge not connected")
    }

    function clearBuffer() {
        // ×键清预览+清真实输入: 一发整框清空(bridge.clearAll), 不循环退格 ——
        // 循环退格按光标位置逐个删, 光标在文本中段时残留光标后内容,
        // 光标在开头时整个无动作。
        buffer = ""
        if (bridge && bridge.clearAll) bridge.clearAll()
        else console.warn("XizzVirtualKeyboard: bridge not connected")
    }
    // feat-329: 面板收起时只清本地预览镜像, 不触碰焦点框真实文本。
    // 与 clearBuffer(×键: 清预览+删真实输入)区分开。
    function clearPreview() {
        buffer = ""
    }
    function commitCandidate(text) { clearBuffer(); commit(text) }

    function submit() {
        if (bridge && bridge.submit) bridge.submit()
        else if (bridge && bridge.hideKeyboard) bridge.hideKeyboard()
    }
}
