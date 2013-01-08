import QtQuick 1.1
import org.LC.common 1.0

Rectangle {
    id: rootRect

    visible: GMN_proxy.msgCount > 0

    width: parent.width
    height: GMN_proxy.msgCount > 0 ? parent.width : 0

    color: "transparent"

    Common { id: commonJS }

    ActionButton {
        id: gmailButton

        anchors.fill: parent
        actionIconURL: "qrc:/gmailnotifier/gmailicon.svg"
        actionIconScales: false

        Text {
            id: numText

            z: parent.z + 2
            anchors.right: parent.right
            anchors.rightMargin: parent.width / 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: parent.height / 10

            text: GMN_proxy.msgCount <= 99 ? GMN_proxy.msgCount : "+"

            color: "white"
            font.pixelSize: parent.height / 3
            font.italic: true
            smooth: true

            verticalAlignment: Text.AlignBottom
            horizontalAlignment: Text.AlignRight
        }

        Rectangle {
            z: numText.z - 1
            anchors.fill: numText
            anchors.topMargin: -2
            anchors.leftMargin: -parent.width / 10
            anchors.bottomMargin: -1
            color: "#E01919"
            smooth: true
            radius: 4
            border.color: "white"
            border.width: 1
        }

        onTriggered: commonJS.showTooltip(rootRect, function(x, y) { GMN_proxy.showMailList(x, y) })
    }
}
