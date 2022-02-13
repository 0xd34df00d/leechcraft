import QtQuick 2.3
import "."

Rectangle {
    id: rootRect

    Item {
        id: bioViewContainer

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: parent.width / 1.618

        BioView {
            anchors.fill: parent
        }
    }

    Item {
        anchors.top: parent.top
        anchors.left: bioViewContainer.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        SimilarView {
            anchors.fill: parent
        }
    }
}
