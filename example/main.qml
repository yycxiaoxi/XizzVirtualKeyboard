// SPDX-License-Identifier: MIT
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import OpenKeyboard 1.0

ApplicationWindow {
    id: win
    width: 800
    height: 480
    visible: true
    title: "OpenKeyboard Example"

    // Background tap: tapping empty area clears focus and dismisses keyboard.
    // Guarded to ignore clicks that land within keyboard bounds (including gaps/margins).
    MouseArea {
        anchors.fill: parent
        z: -1
        enabled: keyboard.active
        propagateComposedEvents: true
        onPressed: {
            if (keyboard.active) {
                var p = mapToItem(keyboard, mouse.x, mouse.y)
                if (p.y >= 0 && p.y <= keyboard.height)
                    { mouse.accepted = false; return }
            }
            mouse.accepted = false
            win.contentItem.forceActiveFocus()
            keyboard.hide()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        anchors.bottomMargin: keyboard.active ? keyboard.height + 16 : 16
        spacing: 12

        TextField { id: f1; Layout.fillWidth: true; placeholderText: "Search (qwerty)"; focus: true }
        TextField { id: f2; Layout.fillWidth: true; placeholderText: "Digits only"; inputMethodHints: Qt.ImhDigitsOnly }
        TextField { id: f3; Layout.fillWidth: true; placeholderText: "Email"; inputMethodHints: Qt.ImhEmailCharactersOnly }
        TextField { id: f4; Layout.fillWidth: true; placeholderText: "Password"; echoMode: TextInput.Password; inputMethodHints: Qt.ImhHiddenText }
        TextArea  { id: f5; Layout.fillWidth: true; Layout.fillHeight: true; placeholderText: "Multi-line" }
    }

    OpenInputPanel {
        id: keyboard
        parent: Overlay.overlay
        anchors.bottom: parent.bottom
        actionLabel: "\u641c\u7d22"
        onActionTriggered: {} // was console.log("action:", text)
    }

    Component.onCompleted: f1.forceActiveFocus()
}
