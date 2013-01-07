import QtQuick 1.1
import org.LC.common 1.0

Rectangle {
    id: rootRect

    width: parent.width
    property real indicatorItemHeight: parent.width
    height: indicatorsView.count * indicatorItemHeight

    color: "transparent"

    ListView {
        id: indicatorsView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds

        model: Lemon_infoModel

        delegate: Rectangle {
            width: indicatorItemHeight
            height: indicatorItemHeight

            color: "transparent"

            ActionButton {
                anchors.fill: parent
                actionIconURL: "image://ThemeIcons/" + iconName + '/' + width
                onTriggered: Lemon_proxy.showGraph(ifaceName)

                Rectangle {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    width: parent.width / 2
                    height: maxDownSpeed ? parent.height * downSpeed / maxDownSpeed : 0

                    color: "#44009900"
                }

                Rectangle {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    width: parent.width / 2
                    height: maxUpSpeed ? parent.height * upSpeed / maxUpSpeed : 0

                    color: "#44990000"
                }
            }
        }
    }
}
