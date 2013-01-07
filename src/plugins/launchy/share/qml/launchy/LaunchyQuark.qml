import QtQuick 1.0
import org.LC.common 1.0

Rectangle {
    id: rootRect

    width: parent.width
    height: width * launchView.count
    color: "transparent"

    ListView {
        id: launchView

        anchors.fill: parent
        model: Launchy_itemModel

        delegate: ActionButton {
            width: rootRect.width
            height: width

            actionIconURL: "image://LaunchyItemIcons/" + permanentID

            onTriggered: Launchy_proxy.launch(permanentID)
        }
    }
}
