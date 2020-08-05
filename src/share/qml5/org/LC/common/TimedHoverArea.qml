import QtQuick 2.0

MouseArea {
    id: mouseAreaItem

    hoverEnabled: true

    property int hoverInTimeout: 0
    signal hoverInTimedOut()
    signal areaExited()

    Timer {
        id: hoverInTimer

        interval: mouseAreaItem.hoverInTimeout
        repeat: false

        onTriggered: mouseAreaItem.hoverInTimedOut()
    }

    onEntered: hoverInTimer.start()
    onExited: {
        if (hoverInTimer.running)
            hoverInTimer.stop();
        mouseAreaItem.areaExited();
    }
}
