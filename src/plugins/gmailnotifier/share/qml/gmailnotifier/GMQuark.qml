import QtQuick 1.1
import "../common/"

Rectangle {
    id: rootRect

    visible: GMN_proxy.msgCount > 0

    width: parent.width
    height: parent.width

    color: "transparent"

    ActionButton {
        anchors.fill: parent
        actionIconURL: "gmail.svg"
        actionIconScales: false

        Text {
            width: parent.width
            height: parent.height

            anchors.right: parent.right
            anchors.rightMargin: width / 10
            anchors.bottom: parent.bottom

            text: GMN_proxy.msgCount
        }
    }
}
