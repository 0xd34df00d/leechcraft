import QtQuick 1.1
import org.LC.common 1.0

Rectangle {
    id: rootRect

    width: parent.width
    property real trayItemHeight: parent.width
    height: trayView.count * trayItemHeight

    radius: 2

    color: "transparent"

    ListView {
        id: trayView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds

        model: SB2_trayModel

        delegate: ActionButton {
            height: rootRect.trayItemHeight
            width: rootRect.width

            isHighlight: actionObject.checked
            actionIconURL: actionIcon
            textTooltip: actionText
            hoverScalesIcons: false

            onTriggered: actionObject.trigger()
        }
    }
}
