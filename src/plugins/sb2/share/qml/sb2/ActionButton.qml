import QtQuick 1.1

Item {
    id: actionRoot

    property bool isHighlight
    property string actionIconURL

    signal triggered()

    Rectangle {
        id: actionRect

        radius: width / 10
        smooth: true

        anchors.fill: parent
        anchors.margins: 2
        border.width: 1
        border.color: actionRoot.isHighlight ? "#a51e00" : "black"

        gradient: Gradient {
            GradientStop {
                position: 1
                color: actionRoot.isHighlight ? "#5a3238" : "#42394b"
            }

            GradientStop {
                position: 0
                color: actionRoot.isHighlight ? "#290700" : "#000000"
            }
        }

        states: [
            State {
                name: "hovered"
                when: actionMouseArea.containsMouse && !actionMouseArea.pressed
                PropertyChanges { target: actionRect; border.color: "white"; anchors.margins: 0 }
            },
            State {
                name: "pressed"
                when: actionMouseArea.containsMouse && actionMouseArea.pressed
                PropertyChanges { target: actionRect; border.color: "black" }
            }
        ]

        transitions: [
            Transition {
                from: ""
                to: "hovered"
                reversible: true
                PropertyAnimation { properties: "border.color,anchors.margins"; duration: 200 }
            }
        ]

        Image {
            id: actionImageElem

            anchors.fill: parent

            source: actionIconURL + '/' + width
            smooth: true
            cache: false
        }

        MouseArea {
            id: actionMouseArea

            anchors.fill: parent
            hoverEnabled: true

            onClicked: actionRoot.triggered()
        }
    }
}
