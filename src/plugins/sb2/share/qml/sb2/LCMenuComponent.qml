import QtQuick 1.1

Rectangle {
    id: rootRect

    width: parent.width
    height: parent.width

    border.width: 1
    border.color: "#333333"
    radius: 2

    color: "transparent"

    ActionButton {
        width: parent.width
        height: parent.width

        onTriggered: SB2_menuComponentProxy.execMenu()
        onHeld: SB2_menuComponentProxy.execMenu()

        actionIconURL: SB2_menuComponentLCIcon
    }
}
