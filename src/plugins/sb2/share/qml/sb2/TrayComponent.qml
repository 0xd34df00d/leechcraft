import QtQuick 1.1
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real itemSize: parent.quarkBaseSize
    property real length: trayView.count * itemSize
    width: viewOrient == "vertical" ? itemSize : length
    height: viewOrient == "vertical" ? length : itemSize

    radius: 2

    color: "transparent"

    ListView {
        id: trayView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds

        model: SB2_trayModel

        orientation: viewOrient == "vertical" ? ListView.Vertical : ListView.Horizontal

        delegate: ActionButton {
            height: rootRect.itemSize
            width: rootRect.itemSize

            isHighlight: actionObject.checked
            actionIconURL: actionIcon
            textTooltip: actionText

            onTriggered: actionObject.trigger()
        }
    }
}
