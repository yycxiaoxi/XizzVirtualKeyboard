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
        else console.warn("OpenKeyboard: bridge not connected")
    }

    function backspace() {
        if (buffer.length > 0) buffer = buffer.slice(0, buffer.length - 1)
        if (bridge && bridge.deleteSurrounding) bridge.deleteSurrounding(1)
        else console.warn("OpenKeyboard: bridge not connected")
    }

    function clearBuffer() {
        // Clear preview and also delete real input content via surrounding text length
        if (buffer.length > 0) {
            // Delete the whole buffer from the focused TextField via repeated backspace
            // Use bridge's signal chain if available, else fallback to local buffer only
            var n = buffer.length
            buffer = ""
            for (var i = 0; i < n; ++i) {
                if (bridge && bridge.deleteSurrounding) bridge.deleteSurrounding(1)
            }
        }
    }
    function commitCandidate(text) { clearBuffer(); commit(text) }
}
