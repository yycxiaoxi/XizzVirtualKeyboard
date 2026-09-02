// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../components"

Item {
    id: root
    implicitHeight: contentCol.implicitHeight
    implicitWidth: contentCol.implicitWidth

    property bool upperCase: false
    signal keyPressed(string value)
    signal submitPressed()
    signal shiftPressed()
    signal backspacePressed()
    signal switchToSymbols()
    signal switchToNumbers()

    // ---- shared hint (alt char) strip ----
    property var _hintKeys: []
    property var _hintSource: null
    property int _hintIndex: 1
    property bool _inHintSession: false
    property bool _hintVisible: false
    property real _hintOriginX: 0

    function registerHintKey(k) { _hintKeys.push(k) }
    function hitTestHintKey(mx, my) {
        for (var i = 0; i < _hintKeys.length; ++i) {
            var k = _hintKeys[i]
            var p = hintTracker.mapToItem(k, mx, my)
            if (p.x >= 0 && p.x <= k.width && p.y >= 0 && p.y <= k.height)
                return k
        }
        return null
    }
    function keyLabel(ch) { return upperCase ? ch.toUpperCase() : ch; }
    function beginHint(item) {
        if (!item || !item.hintText) return
        _hintSource = item
        _inHintSession = true
        _hintVisible = false
        _hintOriginX = item.mapToItem(root, item.width / 2, 0).x
        _hintIndex = 1
        hintTimer.start()
    }
    // End of the shared hint (alt-char) gesture.
    // - If the strip is visible, the three cells already encode the wanted casing
    //   (index 0 = upper, 1 = hint symbol, 2 = lower), so commit via the dedicated
    //   `hintKeyPressed` path which bypasses InputEngine.formatKey's uppercase pass.
    // - Otherwise fall back to the tapped key: emit its raw `keyChar` so the engine's
    //   normal `formatKey` / upperCase handling can apply exactly once.
    signal hintKeyPressed(string value)
    function endHint(commit) {
        hintTimer.stop()
        if (_hintVisible && _hintSource) {
            var base = _hintSource.keyChar.toLowerCase()
            var v = _hintIndex === 0 ? base.toUpperCase() : (_hintIndex === 1 ? _hintSource.hintText : base)
            if (commit && v.length > 0) root.hintKeyPressed(v)
            _hintVisible = false
        } else if (!_hintVisible && _hintSource && commit) {
            root.keyPressed(_hintSource.keyChar)
        }
        _inHintSession = false
        _hintVisible = false
        _hintSource = null
        _hintOriginX = 0
    }

    Timer {
        id: hintTimer
        interval: 220
        repeat: false
        onTriggered: if (root._inHintSession && root._hintSource) { root._hintIndex = 1; root._hintVisible = true }
    }

    ColumnLayout {
        id: contentCol
        anchors.fill: parent
        spacing: 6

        // Row 1: Q W E R T Y U I O P with number hints 1..0 on top
        RowLayout {
            Layout.fillWidth: true
            spacing: 6
            Repeater {
                model: 10
                delegate: TextKey {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 46
                    property int r: index
                    keyChar: root.keyLabel(["q","w","e","r","t","y","u","i","o","p"][r])
                    hintText: ["1","2","3","4","5","6","7","8","9","0"][r]
                    Component.onCompleted: root.registerHintKey(this)
                }
            }
        }
        // Row 2: A S D F G H J K L with symbol hints
        RowLayout {
            Layout.fillWidth: true
            spacing: 6
            Repeater {
                model: 9
                delegate: TextKey {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 46
                    property int r: index
                    keyChar: root.keyLabel(["a","s","d","f","g","h","j","k","l"][r])
                    hintText: ["~","!","@","#","%","^","\u0027","\"","?"][r]
                    Component.onCompleted: root.registerHintKey(this)
                }
            }
        }
        // Row 3: ↑ (shift) Z X C V B N M ⌫
        RowLayout {
            Layout.fillWidth: true
            spacing: 6
            FunctionKey {
                Layout.preferredWidth: 52
                Layout.fillHeight: true
                keyText: "\u2B06"
                active: root.upperCase
                normalColor: root.upperCase ? "#7ED321" : "#C9CDD3"
                textColor: root.upperCase ? "#FFFFFF" : "#1A1A1A"
                onClicked: root.shiftPressed()
            }
            Repeater {
                model: ["z","x","c","v","b","n","m"]
                delegate: TextKey {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 46
                    property int r: index
                    keyChar: root.keyLabel(modelData)
                    hintText: ["(",")","-","_",":",";","/"][r]
                    Component.onCompleted: root.registerHintKey(this)
                }
            }
            FunctionKey {
                Layout.preferredWidth: 52
                Layout.preferredHeight: 46
                keyText: "\u232b"
                autoRepeat: true
                autoRepeatDelay: 520
                onPressed: root.backspacePressed()
            }
        }
        // Row 4: !?# | 123 | space | . | \u21a9  - fills width, no side gaps
        RowLayout {
            Layout.fillWidth: true
            spacing: 6
            Layout.preferredHeight: 48
            FunctionKey {
                Layout.fillWidth: true
                Layout.preferredWidth: 52
                Layout.fillHeight: true
                keyText: "!?#"
                onClicked: root.switchToSymbols()
            }
            FunctionKey {
                Layout.fillWidth: true
                Layout.preferredWidth: 68
                Layout.fillHeight: true
                keyText: "123"
                onClicked: root.switchToNumbers()
            }
            SpaceMicKey {
                Layout.fillWidth: true
                Layout.preferredWidth: 120
                Layout.fillHeight: true
                onPressed: root.keyPressed(" ")
            }
            FunctionKey {
                Layout.preferredWidth: 52
                Layout.fillHeight: true
                keyText: "."
                onPressed: root.keyPressed(".")
            }
                    EnterKey {
            Layout.fillWidth: true
            Layout.preferredWidth: 78
            Layout.fillHeight: true
            onPressed: root.submitPressed()
        }
        }
    }

    // Fixed alt strip above the pressed key (not following finger)
    Item {
        id: hintStrip
        visible: root._hintVisible && root._hintSource !== null
        width: 3 * 46 + 2 * 6   // 3 cells (upper / key / lower)
        height: 48
        z: 100
        x: root._hintSource ? Math.max(4, Math.min(root.width - width - 4, root._hintSource.mapToItem(root, 0, 0).x + (root._hintSource.width - width) / 2)) : 0
        y: root._hintSource ? (root._hintSource.mapToItem(root, 0, 0).y - height - 8) : 0
        Row {
            anchors.fill: parent
            spacing: 6
            Repeater {
                model: 3
                delegate: Rectangle {
                    width: 46; height: 48; radius: 10
                    color: root._hintIndex === index ? "#2F80ED" : "#FFFFFF"
                    border.color: root._hintIndex === index ? "#2F80ED" : "#E0E4EA"; border.width: 1
                    Text {
                        anchors.centerIn: parent
                        text: index === 0 ? (root._hintSource ? root._hintSource.keyChar.toLowerCase().toUpperCase() : "") : (index === 1 ? (root._hintSource ? root._hintSource.hintText : "") : (root._hintSource ? root._hintSource.keyChar.toLowerCase() : ""))
                        font.pixelSize: 19; font.weight: Font.Medium
                        color: root._hintIndex === index ? "#FFFFFF" : "#1A1A1A"
                    }
                }
            }
        }
    }

    // Press-owning tracker: takes the press on a hint key at the very first moment,
    // so it keeps the grab and receives all moves/releases (this is the key fix —
    // a MouseArea that becomes visible later cannot steal an already-grabbed press).
    MouseArea {
        id: hintTracker
        anchors.fill: parent
        z: 9999
        hoverEnabled: true
        propagateComposedEvents: true
        onPressed: {
            var k = root.hitTestHintKey(mouse.x, mouse.y)
            if (k) { root.beginHint(k); return }
            mouse.accepted = false
        }
        onPositionChanged: {
            if (!root._inHintSession) return
            if (root._hintVisible) {
                var d = mouse.x - root._hintOriginX
                if (d < -15) root._hintIndex = 0
                else if (d > 15) root._hintIndex = 2
                else root._hintIndex = 1
            }
        }
        onReleased: {
            if (root._inHintSession) { root.endHint(true); return }
            mouse.accepted = false
        }
        onCanceled: if (root._inHintSession) root.endHint(false)
    }
}
