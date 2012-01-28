import QtQuick 1.0

Item {
    id: scrollBar

    property real position
    property real pgSize
    property variant orientation : Qt.Vertical

    Rectangle {
        id: background

        anchors.fill: parent

        radius: orientation == Qt.Vertical ? (width / 2 - 1) : (height / 2 -1)
        border.color: "#000000"
        color: "white"
        opacity: 0.3
    }

    Rectangle {
        x: orientation == Qt.Vertical ? 1 : (scrollBar.position * (scrollBar.width - 2) + 1)
        y: orientation == Qt.Vertical ? (scrollBar.position * (scrollBar.height - 2) + 1) : 1
        width: orientation == Qt.Vertical ? (parent.width - 2) : (scrollBar.pgSize * (scrollBar.width - 2))
        height: orientation == Qt.Vertical ? (scrollBar.pgSize * (scrollBar.height - 2)) : (parent.height - 2)
        radius: orientation == Qt.Vertical ? (width / 2 - 1) : (height / 2 - 1)
        color: "black"
        opacity: 0.7
    }
}
