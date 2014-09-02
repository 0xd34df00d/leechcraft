import QtQuick 2.3
import org.LC.common 1.0

Rectangle {
    id: rootRect

    visible: GMN_proxy.msgCount > 0

    implicitWidth: GMN_proxy.msgCount > 0 ? parent.quarkBaseSize : 0
    implicitHeight: width

    color: "transparent"

    Common { id: commonJS }

    ActionButton {
        id: gmailButton

        anchors.fill: parent
        actionIconURL: "qrc:/gmailnotifier/gmailicon.svg"
        actionIconScales: false

        overlayText: GMN_proxy.msgCount <= 99 ? GMN_proxy.msgCount : "+"

        onTriggered: commonJS.showTooltip(rootRect, function(x, y) { GMN_proxy.showMailList(x, y, quarkProxy.getWinRect()) })
    }
}
