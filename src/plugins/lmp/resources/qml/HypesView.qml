import QtQuick 1.0
import Effects 1.0
import "."

Rectangle {
    id: rootRect
    anchors.fill: parent

    color: "#000000"

    Rectangle {
        id: artistsRect

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width / 2

        color: "#000000"

        Rectangle {
            id: artistsLabel

            color: "#53485F"

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: artistNameLabel.height

            Text {
                id: artistNameLabel
                z: 2
                text: artistsLabelText
                font.bold: true
                font.pointSize: 14
                color: "#dddddd"
                anchors.centerIn: parent
            }
        }

        Rectangle {
            anchors.top: artistsLabel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            clip: true

            SimilarView {
                model: artistsModel
            }
        }
    }

    Rectangle {
        anchors.left: artistsRect.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        color: "#000000"

        Rectangle {
            id: tracksLabel

            color: "#53485F"

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: artistNameLabel.height

            Text {
                id: tracksNameLabel
                z: 2
                text: tracksLabelText
                font.bold: true
                font.pointSize: 14
                color: "#dddddd"
                anchors.centerIn: parent
            }
        }

        Rectangle {
            anchors.top: tracksLabel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            clip: true

            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#53485F"
                }

                GradientStop {
                    position: 1
                    color: "#222222"
                }
            }
        }
    }
}
