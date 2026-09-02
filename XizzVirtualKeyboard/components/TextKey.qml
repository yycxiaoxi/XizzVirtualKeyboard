// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
import QtQuick 2.15
import QtQuick.Controls 2.15

KeyButton {
    id: root
    property string keyChar: ""
    property string hintText: ""
    keyText: keyChar

    signal hintPicked(string value)

    // Simple tap: deferred to hintPicked on release for uniform handling.
    // Long-press is now managed by parent QwertyLayout's shared strip, so TextKey just emits hintPicked on short release.
    // Keep a lightweight local press flag so short tap still works when not handled by parent.
    property bool _shortArmed: false
    onPressedChanged: {
        if (pressed && hintText.length > 0) _shortArmed = true
        else if (!pressed && _shortArmed && !parent._inHintSession) {
            _shortArmed = false
            root.hintPicked(keyChar)
        } else if (!pressed) { _shortArmed = false }
    }

    // Visual will hide while shared strip is active (parent dims it)
    opacity: parent && parent._inHintSession && parent._hintSource === root ? 0.0 : 1.0
    Behavior on opacity { NumberAnimation { duration: 80 } }

    contentItem: Item {
        Column {
            anchors.centerIn: parent
            spacing: hintLabel.visible ? 1 : 0
            Text {
                id: hintLabel
                visible: root.hintText.length > 0
                text: root.hintText
                font.pixelSize: 9; color: "#8A9099"
                horizontalAlignment: Text.AlignHCenter; anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: root.keyText
                font.pixelSize: 18; font.weight: Font.Medium; color: root.textColor
                horizontalAlignment: Text.AlignHCenter; anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
