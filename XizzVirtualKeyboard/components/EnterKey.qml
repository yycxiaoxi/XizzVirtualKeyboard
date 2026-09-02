// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
import QtQuick 2.15

KeyButton {
    id: root
    normalColor: "#00C7A0"
    pressedColor: "#00B38F"
    textColor: "#FFFFFF"

    contentItem: Image {
        source: Qt.resolvedUrl("../icons/enter.svg")
        sourceSize.width: 96
        sourceSize.height: 96
        fillMode: Image.PreserveAspectFit
        smooth: false
        mipmap: false
        anchors.centerIn: parent
    }
}
