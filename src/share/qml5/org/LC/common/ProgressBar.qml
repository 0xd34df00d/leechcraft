import QtQuick 2.0

Item {
    id: rootItem

    property int minimum: 0
    property int maximum: 100
    property int value: 0
    property color color: "#77B753"

    width: 400
    height: 13
    clip: true

    Rectangle {
        id: border
        anchors.fill: parent
        anchors.bottomMargin: 1
        anchors.rightMargin: 1
        color: "transparent"
        border.width: 1
        border.color: parent.color
    }

    Rectangle {
        id: highlight
        width: rootItem.width * (value - minimum) / (maximum - minimum)

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: parent.color
    }
}
