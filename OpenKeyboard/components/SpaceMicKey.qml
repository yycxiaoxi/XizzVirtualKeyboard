// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
import QtQuick 2.15

KeyButton {
    id: root
    normalColor: "#FFFFFF"
    pressedColor: "#E8ECF0"
    textColor: "#1A1A1A"

    contentItem: Image {
        source: Qt.resolvedUrl("../icons/space.svg")
        sourceSize.width: 42
        sourceSize.height: 16
        fillMode: Image.PreserveAspectFit
        anchors.centerIn: parent
    }
}
