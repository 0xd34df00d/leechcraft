import QtQuick 1.1
import "."

Rectangle {
    id: rootRect

    width: parent.width
    property real launcherItemHeight: parent.width
    height: launcherView.count * launcherItemHeight

    color: "transparent"

    ListView {
        id: launcherView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds

        model: SB2_launcherModel

        delegate: ActionButton {
            height: rootRect.trayItemHeight
            width: rootRect.width

            actionIconURL: actionIcon
        }
    }
}
