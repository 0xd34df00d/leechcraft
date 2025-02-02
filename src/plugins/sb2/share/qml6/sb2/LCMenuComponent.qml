import QtQuick
import org.LC.common 1.0

Rectangle {
    id: rootRect

    implicitWidth: parent.quarkBaseSize
    implicitHeight: parent.quarkBaseSize

    radius: 2

    color: "transparent"

    ActionButton {
        width: parent.width
        height: parent.width

        onTriggered: menuComponentProxy.execMenu()
        onHeld: menuComponentProxy.execMenu()

        actionIconURL: menuComponentLCIcon
        textTooltip: menuTooltipString
    }
}
