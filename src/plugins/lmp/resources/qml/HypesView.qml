import QtQuick 1.0
import Effects 1.0
import "."

Rectangle {
    id: rootRect
    anchors.fill: parent

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width / 2

        SimilarView {
            model: artistsModel
        }
    }
}
