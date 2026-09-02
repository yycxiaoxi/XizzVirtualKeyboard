// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
import QtQuick 2.15
import QtQuick.Controls 2.15

AbstractButton {
    id: root
    property alias keyText: label.text
    property color normalColor: "#FFFFFF"
    property color pressedColor: "#E8ECF0"
    property color textColor: "#1A1A1A"
    property int keyRadius: 8

    focusPolicy: Qt.NoFocus
    activeFocusOnTab: false
    focus: false
    autoRepeat: false
    autoRepeatInterval: 55
    autoRepeatDelay: 520

    padding: 0
    implicitHeight: 46
    implicitWidth: 40

    scale: pressed ? 0.97 : 1.0
    Behavior on scale { NumberAnimation { duration: 45 } }

    background: Rectangle {
        radius: root.keyRadius
        color: root.pressed ? root.pressedColor : root.normalColor
        border.color: "#E0E4EA"
        border.width: 0
    }

    contentItem: Text {
        id: label
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 20
        color: root.textColor
        elide: Text.ElideRight
    }
}
