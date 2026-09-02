// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import "components"
import "layouts"

Rectangle {
    id: root
    focus: false
    color: "#EDEFF2"
    clip: false
    implicitHeight: mainCol.implicitHeight + 16

    property alias engine: inputEngine
    property bool showCandidateBar: true
    // ImhHiddenText (Password) — keyboard stays open, preview is masked (bridge is authoritative)
    property bool isPassword: bridge ? bridge.isPassword : false

    property var bridge: null
    InputEngine { id: inputEngine; bridge: root.bridge }

    function maskedText(t) {
        if (!t) return ""
        var s = ""
        for (var i = 0; i < t.length; ++i) s += "•"
        return s
    }

    ColumnLayout {
        id: mainCol
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        // Top row: white input preview bar (left, fills) + collapse button (right, outside box)
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Rectangle {
                id: bufferDisplay
                Layout.fillWidth: true
                Layout.preferredHeight: 38
                radius: 8
                color: "#FFFFFF"
                border.color: "#E0E4EA"
                border.width: 1
                clip: true
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 8
                    Text {
                        Layout.fillWidth: true
                        text: root.isPassword
                              ? root.maskedText(root.bridge ? root.bridge.surroundingText : "")
                              : (root.bridge ? root.bridge.surroundingText : "")
                        font.pixelSize: 15
                        color: "#1A1A1A"
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                    ToolButton {
                        focusPolicy: Qt.NoFocus
                        activeFocusOnTab: false
                        visible: root.bridge ? root.bridge.surroundingText.length > 0 : false
                        text: "\u00d7"
                        font.pixelSize: 18
                        onClicked: inputEngine.clearBuffer()
                        background: Rectangle { color: "transparent" }
                    }
                }
            }
            // Collapse button beside the white bar (not framed inside)
            ToolButton {
                Layout.preferredWidth: 38
                Layout.preferredHeight: 38
                focusPolicy: Qt.NoFocus
                activeFocusOnTab: false
                text: "\u25bc"
                font.pixelSize: 14
                onClicked: { if (root.bridge && root.bridge.hideKeyboard) root.bridge.hideKeyboard() }
                background: Rectangle { radius: 8; color: "#E8EDF2"; border.color: "#E0E4EA"; border.width: 1 }
            }
        }

        CandidateBar {
            Layout.fillWidth: true
            visible: false // removed per request (candidate strip deleted)
            // visible: root.showCandidateBar
            candidates: inputEngine.candidates
            onCandidateClicked: inputEngine.commitCandidate(text)
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: inputEngine.page

            QwertyLayout {
                upperCase: inputEngine.upperCase
                onKeyPressed: inputEngine.commit(value)
                onHintKeyPressed: inputEngine.commitRaw(value)
                onSubmitPressed: inputEngine.submit()
                onShiftPressed: inputEngine.toggleShift()
                onBackspacePressed: inputEngine.backspace()
                onSwitchToSymbols: inputEngine.page = 1
                onSwitchToNumbers: inputEngine.page = 2
            }
            SymbolsLayout {
                onKeyPressed: inputEngine.commit(value)
                onSubmitPressed: inputEngine.submit()
                onBackspacePressed: inputEngine.backspace()
                onSwitchBack: inputEngine.page = 0
            }
            NumberLayout {
                onKeyPressed: inputEngine.commit(value)
                onSubmitPressed: inputEngine.submit()
                onBackspacePressed: inputEngine.backspace()
                onSwitchBack: inputEngine.page = 0
                onSwitchToSymbols: inputEngine.page = 1
            }
        }

    }

    function applyHints(hints) {
        var pw = (hints & Qt.ImhHiddenText) || (bridge && bridge.isPassword)
        if (pw) {
            inputEngine.page = 0
            return
        }
        if ((hints & Qt.ImhDigitsOnly) || (hints & Qt.ImhFormattedNumbersOnly)) {
            inputEngine.page = 2
        } else if ((hints & Qt.ImhEmailCharactersOnly) || (hints & Qt.ImhDialableCharactersOnly)) {
            inputEngine.page = 0
        } else {
            if (inputEngine.page === 2) inputEngine.page = 0
        }
    }
}
