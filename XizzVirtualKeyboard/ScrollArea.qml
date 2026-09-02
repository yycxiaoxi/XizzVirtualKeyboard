// SPDX-License-Identifier: MIT
// Copyright (c) 2026 MTL
import QtQuick 2.15
import QtQuick.Window 2.15

// ScrollArea — keyboard-aware scrollable container shipped with
// XizzVirtualKeyboard.
//
// Hosts put the content that must stay reachable while the virtual keyboard
// is open into this container. When the keyboard pops up (the panel is a pure
// overlay on Qt5/Quick, nothing pans the host content automatically) the
// viewport shrinks so its bottom edge sits above the keyboard, and the
// currently focused input item is scrolled into the visible band. When the
// keyboard hides the viewport returns to the full host height.
//
// It is intentionally style-free: no background, no forced anchors. Give it
// the size you want (usually anchors.fill of the page/dialog body) and set
// contentHeight if your content is taller than the resting viewport.
//
// The same "reveal the focused field on popup" job is also done from C++ by
// XizzVirtualKeyboardInputContext::scrollFocusInputIntoView(); this component
// additionally handles the viewport shrink itself, so it works even without
// the C++ auto-scroll enabled.
Item {
    id: root

    // Content is written straight into the inner Flickable (default property),
    // so child items land on the scrollable content item as usual.
    default property alias data: flick.data

    // Forwarded Flickable knobs used most often by hosts.
    property alias contentWidth: flick.contentWidth
    property alias contentHeight: flick.contentHeight
    property alias contentX: flick.contentX
    property alias contentY: flick.contentY
    property alias clip: flick.clip
    property alias interactive: flick.interactive

    // Extra breathing room kept below the focused field when it is scrolled
    // into view (device pixels).
    property real revealMargin: 12

    readonly property bool keyboardVisible: typeof Qt.inputMethod !== "undefined" && Qt.inputMethod.visible

    // Top edge of the keyboard in this window's coordinates.
    readonly property real keyboardTop: {
        if (!keyboardVisible)
            return root.height // no keyboard: no shrink below
        var kbr = Qt.inputMethod.keyboardRectangle
        return kbr.y
    }

    // Height the inner viewport is allowed to use (shrinks while the keyboard
    // overlaps this item's area).
    property real _viewHeight: root.height

    function _recomputeViewHeight() {
        // Map to window coordinates (mapToItem(null, ...) == scene coords,
        // which match Qt.inputMethod.keyboardRectangle for a fullscreen window).
        var topInWindow = root.mapToItem(null, 0, 0).y
        var available = root.keyboardTop - topInWindow
        if (!root.keyboardVisible)
            available = root.height
        _viewHeight = Math.max(0, Math.min(root.height, available))
    }

    // Scroll so the focused input item (when it lives inside our content)
    // is fully visible in the current (possibly shrunk) viewport.
    function ensureFocusedVisible() {
        var win = root.Window.window
        if (!win)
            return
        var f = win.activeFocusItem
        if (!f)
            return
        var inside = false
        var c = f
        while (c) {
            if (c === flick) { inside = true; break }
            c = c.parent
        }
        if (!inside)
            return
        var itemY = f.mapToItem(flick.contentItem, 0, 0).y
        var itemBottom = itemY + f.height
        var pad = root.revealMargin
        var cy = flick.contentY
        if (itemY < cy + pad)
            cy = itemY - pad
        else if (itemBottom > cy + flick.height - pad)
            cy = itemBottom - flick.height + pad
        cy = Math.max(0, Math.min(cy, flick.contentHeight - flick.height))
        if (cy !== flick.contentY)
            flick.contentY = cy
    }

    function _onKeyboardState() {
        _recomputeViewHeight()
        // Layout the shrunk viewport before measuring the focused field.
        Qt.callLater(function() { root.ensureFocusedVisible() })
    }

    Connections {
        target: Qt.inputMethod
        function onVisibleChanged() { root._onKeyboardState() }
        function onKeyboardRectangleChanged() { root._onKeyboardState() }
    }

    onWidthChanged: _recomputeViewHeight()
    onHeightChanged: _recomputeViewHeight()
    onParentChanged: Qt.callLater(function() { root._recomputeViewHeight() })

    Component.onCompleted: Qt.callLater(function() { root._recomputeViewHeight() })

    Flickable {
        id: flick
        x: 0
        y: 0
        width: root.width
        height: root._viewHeight
        contentWidth: root.width
        // Default: page-sized content fully reachable at rest; hosts with
        // taller content override contentHeight (or set content items' own).
        contentHeight: root.height
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick
        clip: true
    }
}
