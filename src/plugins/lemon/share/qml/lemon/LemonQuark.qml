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

            Image {
                anchors.fill: parent
                source: "image://ThemeIcons/" + iconName + '/' + width

                effect: Colorize {
                    strength: isActive ? 0 : 0.5
                    color: "green"

                    Behavior on strength { PropertyAnimation {} }
                }
            }
        }
    }
}
