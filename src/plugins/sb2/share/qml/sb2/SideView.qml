import QtQuick 1.1

Rectangle {
    id: rootRect
    color: "black"
    anchors.fill: parent

    ListView {
        id: itemsView

        anchors.fill: parent

        model: itemsModel

        delegate: Rectangle {
            id: itemsDelegate

            height: itemLoader.height
            width: itemsView.width

            color: "transparent"

            Loader {
                id: itemLoader

                source: sourceURL
                height: item.height
                width: parent.width

                clip: true
            }
        }
    }
}
