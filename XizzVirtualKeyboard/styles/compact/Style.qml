// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
pragma Singleton
import QtQuick 2.15

QtObject {
    readonly property color bg: "#EDEFF2"
    readonly property color keyBg: "#FFFFFF"
    readonly property color funcBg: "#C9CDD3"
    readonly property color funcBgPressed: "#B8BDC6"
    readonly property color accentBg: "#00C7A0"
    readonly property color accentPressed: "#00B38F"
    readonly property color borderColor: "#E0E4EA"
    readonly property color textColor: "#1A1A1A"
    readonly property color accentTextColor: "#FFFFFF"
    readonly property color shiftActive: "#7ED321"
    readonly property color candidateBg: "#FFFFFF"
    readonly property color candidateActive: "#00C7A0"
    readonly property int radius: 8
    readonly property int keyRadius: 8
    readonly property int spacing: 6
    readonly property int outerMargin: 8
    readonly property int candidateHeight: 36
    readonly property int keyHeight: 46
    readonly property int bottomRowHeight: 48
    readonly property int keyFontSize: 20
    readonly property int funcFontSize: 16
    readonly property int actionFontSize: 18
    readonly property int candidateFontSize: 15
}
