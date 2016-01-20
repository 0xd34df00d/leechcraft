import QtQuick 2.3
import QtQuick.Controls.Styles 1.4
import org.LC.common 1.0

ScrollViewStyle {
    id: scrollStyle
    property real dim: 12
    scrollBarBackground: Rectangle {
        implicitWidth: scrollStyle.dim
        implicitHeight: scrollStyle.dim
        color: "transparent"
    }

    incrementControl: ActionButton {
        actionIconURL: "image://ThemeIcons/arrow-down/" + width
        implicitWidth: scrollStyle.dim
        implicitHeight: scrollStyle.dim

        forceHover: styleData.hovered
        forcePress: styleData.pressed
    }

    decrementControl: ActionButton {
        actionIconURL: "image://ThemeIcons/arrow-up/" + width
        implicitWidth: scrollStyle.dim
        implicitHeight: scrollStyle.dim

        forceHover: styleData.hovered
        forcePress: styleData.pressed
    }

    handle: ActionButton {
        implicitWidth: scrollStyle.dim
        implicitHeight: scrollStyle.dim

        forceHover: styleData.hovered
        forcePress: styleData.pressed

        isCurrent: true
    }
}
