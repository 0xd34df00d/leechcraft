import QtQuick 1.1
import "../common/"
import "../common/Common.js" as Common

Rectangle {
    id: rootRect

    visible: GMN_proxy.msgCount > 0

    width: parent.width
    height: GMN_proxy.msgCount > 0 ? parent.width : 0

    color: "transparent"

    ActionButton {
        anchors.fill: parent
        actionIconURL: "qrc:/gmailnotifier/gmailicon.svg"
        actionIconScales: false

        Text {
            width: parent.width
            height: parent.height

            anchors.right: parent.right
            anchors.rightMargin: width / 10
            anchors.bottom: parent.bottom

            text: GMN_proxy.msgCount <= 99 ? GMN_proxy.msgCount : "+"

            color: "#000080"
            font.pixelSize: height * 2 / 3
            smooth: true

            verticalAlignment: Text.AlignBottom
            horizontalAlignment: Text.AlignRight
        }

        onTriggered: Common.showTooltip(rootRect, function(x, y) { GMN_proxy.showMailList(x, y) })
    }
}
