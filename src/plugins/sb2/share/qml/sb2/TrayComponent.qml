import QtQuick 1.1

Rectangle {
    id: rootRect

    width: parent.width
    property real trayItemHeight: parent.width
    height: trayView.count * trayItemHeight

    ListView {
        id: trayView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds

        model: trayModel

        delegate: Rectangle {
            id: trayViewDelegate

            height: rootRect.trayItemHeight
            width: rootRect.width

            border.width: 1
            border.color: "black"

            Image {
                id: actionImageElem
                anchors.fill: parent
                source: actionIcon + '/' + width
                smooth: true
            }
        }
    }
}
