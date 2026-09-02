// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../components"

ColumnLayout {
    id: root
    spacing: 6
    signal keyPressed(string value)
    signal backspacePressed()
    signal switchBack()

    // Row 1: 1 2 3 4 5 6 7 8 9 0  (screenshot 2 top)
    RowLayout {
        Layout.fillWidth: true
        spacing: 6
        Repeater {
            model: ["1","2","3","4","5","6","7","8","9","0"]
            delegate: TextKey {
                Layout.fillWidth: true
                Layout.preferredHeight: 46
                keyChar: modelData
                onPressed: root.keyPressed(keyChar)
            }
        }
    }
    // Row 2: @ # $ _ & - + ( ) /   (screenshot 2 second row: note dash variants)
    RowLayout {
        Layout.fillWidth: true
        spacing: 6
        Repeater {
            model: ["@","#","$","_","&","-","+","(",")","/"]
            delegate: TextKey {
                Layout.fillWidth: true
                Layout.preferredHeight: 46
                keyChar: modelData
                onPressed: root.keyPressed(keyChar)
            }
        }
    }
    // Row 3: * " ' : ; ! ?  ⌫
    RowLayout {
        Layout.fillWidth: true
        spacing: 6
        Repeater {
            model: ["*","\"","\u0027",":",";","!","?"]
            delegate: TextKey {
                Layout.fillWidth: true
                Layout.preferredHeight: 46
                keyChar: modelData
                onPressed: root.keyPressed(keyChar)
            }
        }
        FunctionKey {
            Layout.preferredWidth: 52
            Layout.fillHeight: true
            keyText: "\u232b"
            autoRepeat: true
            autoRepeatDelay: 520
            onPressed: root.backspacePressed()
        }
    }
    // Row 4: (centered) ABC | 123 | ， | space(---) | 。 | ↵  (screenshot 2 bottom)
    RowLayout {
        Layout.fillWidth: true
        spacing: 6
        Layout.preferredHeight: 48
        FunctionKey {
            Layout.fillWidth: true
            Layout.preferredWidth: 52
            Layout.fillHeight: true
            keyText: "ABC"
            onClicked: root.switchBack()
        }
        FunctionKey {
            Layout.fillWidth: true
            Layout.preferredWidth: 52
            Layout.fillHeight: true
            keyText: "123"
            onClicked: root.switchBack()
        }
        FunctionKey {
            Layout.preferredWidth: 46
            Layout.fillHeight: true
            keyText: "\uff0c"
            onPressed: root.keyPressed("\uff0c")
        }
        SpaceMicKey {
            Layout.fillWidth: true
            Layout.preferredWidth: 180
            Layout.fillHeight: true
            onPressed: root.keyPressed(" ")
        }
        FunctionKey {
            Layout.preferredWidth: 46
            Layout.fillHeight: true
            keyText: "\u00b7"
            onPressed: root.keyPressed("\u00b7")
        }
                EnterKey {
            Layout.fillWidth: true
            Layout.preferredWidth: 78
            Layout.fillHeight: true
            onPressed: root.keyPressed("
")
        }
    }
}
