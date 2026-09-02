// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    height: 36
    color: "#FFFFFF"
    clip: true

    property var candidates: ["大家都在做", "美味上新"]
    signal candidateClicked(string text)

    Row {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 24

        Repeater {
            model: root.candidates
            delegate: AbstractButton {
                focusPolicy: Qt.NoFocus
                activeFocusOnTab: false
                height: root.height
                onClicked: root.candidateClicked(modelData)
                contentItem: Text {
                    text: modelData
                    font.pixelSize: 15
                    color: index === 0 ? "#00C7A0" : "#666666"
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle { color: "transparent" }
            }
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: "#E0E4EA"
    }
}
