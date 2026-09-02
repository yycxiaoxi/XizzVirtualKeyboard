// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
pragma Singleton
import QtQuick 2.15

QtObject {
    readonly property color bg: "#F5F5F5"
    readonly property color keyBg: "#FFFFFF"
    readonly property color funcBg: "#D8D8D8"
    readonly property color funcBgPressed: "#C0C0C0"
    readonly property color accentBg: "#3A7BFF"
    readonly property color accentPressed: "#2E62CC"
    readonly property color borderColor: "#C8C8C8"
    readonly property color textColor: "#212121"
    readonly property color accentTextColor: "#FFFFFF"
    readonly property color shiftActive: "#4CAF50"
    readonly property color candidateBg: "#FFFFFF"
    readonly property color candidateActive: "#3A7BFF"
    readonly property int radius: 6
    readonly property int keyRadius: 6
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
