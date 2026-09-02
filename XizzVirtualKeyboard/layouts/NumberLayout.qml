// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../components"

ColumnLayout {
    id: root
    spacing: 6
    signal keyPressed(string value)
    signal submitPressed()
    signal backspacePressed()
    signal switchBack()
    signal switchToSymbols()

    // Nine-grid like screenshot 3, with side function columns
    // Row 1: % | 1 2 3 | ⌫
    RowLayout {
        Layout.fillWidth: true
        spacing: 6
        FunctionKey { Layout.preferredWidth: 52; Layout.fillHeight: true; Layout.preferredHeight: 46; keyText: "%"; onPressed: root.keyPressed("%") }
        TextKey { Layout.fillWidth: true; Layout.preferredHeight: 46; keyChar: "1"; onPressed: root.keyPressed(keyChar) }
        TextKey { Layout.fillWidth: true; Layout.preferredHeight: 46; keyChar: "2"; onPressed: root.keyPressed(keyChar) }
        TextKey { Layout.fillWidth: true; Layout.preferredHeight: 46; keyChar: "3"; onPressed: root.keyPressed(keyChar) }
        FunctionKey { Layout.preferredWidth: 52; Layout.fillHeight: true; Layout.preferredHeight: 46; keyText: "\u232b"; autoRepeat: true; onPressed: root.backspacePressed(); onClicked: root.backspacePressed() }
    }
    // Row 2: : | 4 5 6 | .
    RowLayout {
        Layout.fillWidth: true
        spacing: 6
        FunctionKey { Layout.preferredWidth: 52; Layout.fillHeight: true; Layout.preferredHeight: 46; keyText: ":"; onPressed: root.keyPressed(":") }
        TextKey { Layout.fillWidth: true; Layout.preferredHeight: 46; keyChar: "4"; onPressed: root.keyPressed(keyChar) }
        TextKey { Layout.fillWidth: true; Layout.preferredHeight: 46; keyChar: "5"; onPressed: root.keyPressed(keyChar) }
        TextKey { Layout.fillWidth: true; Layout.preferredHeight: 46; keyChar: "6"; onPressed: root.keyPressed(keyChar) }
        FunctionKey { Layout.preferredWidth: 52; Layout.fillHeight: true; Layout.preferredHeight: 46; keyText: "\u00b7"; onPressed: root.keyPressed(".") }
    }
    // Row 3: - | 7 8 9 | @
    RowLayout {
        Layout.fillWidth: true
        spacing: 6
        FunctionKey { Layout.preferredWidth: 52; Layout.fillHeight: true; Layout.preferredHeight: 46; keyText: "-"; onPressed: root.keyPressed("-") }
        TextKey { Layout.fillWidth: true; Layout.preferredHeight: 46; keyChar: "7"; onPressed: root.keyPressed(keyChar) }
        TextKey { Layout.fillWidth: true; Layout.preferredHeight: 46; keyChar: "8"; onPressed: root.keyPressed(keyChar) }
        TextKey { Layout.fillWidth: true; Layout.preferredHeight: 46; keyChar: "9"; onPressed: root.keyPressed(keyChar) }
        FunctionKey { Layout.preferredWidth: 52; Layout.fillHeight: true; Layout.preferredHeight: 46; keyText: "@"; onPressed: root.keyPressed("@") }
    }
    // Row 4: bottom bar 5-column grid, 0 aligned under 2/5/8
    GridLayout {
        columns: 5
        columnSpacing: 6
        rowSpacing: 6
        FunctionKey { Layout.fillWidth: true; Layout.preferredHeight: 48; keyText: "!*#"; onClicked: root.switchToSymbols() }
        FunctionKey { Layout.fillWidth: true; Layout.preferredHeight: 48; keyText: "\u8fd4\u56de"; onClicked: root.switchBack() }
        TextKey { Layout.fillWidth: true; Layout.preferredHeight: 48; keyChar: "0"; onPressed: root.keyPressed(keyChar) }
        SpaceMicKey { Layout.fillWidth: true; Layout.preferredHeight: 48; onPressed: root.keyPressed(" ") }
        EnterKey { Layout.fillWidth: true; Layout.preferredHeight: 48; onPressed: root.submitPressed() }
    }
}
