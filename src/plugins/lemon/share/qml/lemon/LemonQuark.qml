import QtQuick 1.1
import Effects 1.0

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

            Rectangle {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                width: parent.width / 2
                height: maxDownSpeed ? parent.height * downSpeed / maxDownSpeed : 0

                color: "#bb009900"
            }

            Rectangle {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                width: parent.width / 2
                height: maxUpSpeed ? parent.height * upSpeed / maxUpSpeed : 0

                color: "#bb990000"
            }

            Image {
                anchors.fill: parent
                source: "image://ThemeIcons/" + iconName + '/' + width
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true

                onHovered: Lemon_proxy.showGraph(ifaceName)
            }
        }
    }
}
