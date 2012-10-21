import QtQuick 1.1

Rectangle {
    id: rootRect

    width: parent.width
    property real trayItemHeight: parent.width
    height: trayView.count * trayItemHeight

    color: "transparent"

    ListView {
        id: trayView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds

        model: SB2_trayModel

        delegate: Rectangle {
            id: trayViewDelegate

            height: rootRect.trayItemHeight
            width: rootRect.width
            radius: width / 10
            smooth: true

            border.width: 1
            border.color: "black"

            gradient: Gradient {
                GradientStop {
                    position: 1
                    color: "#42394b"
                }

                GradientStop {
                    position: 0
                    color: "#000000"
                }
            }

            Image {
                id: actionImageElem
                anchors.fill: parent
                source: actionIcon + '/' + width
                smooth: true
            }

            MouseArea {
                anchors.fill: parent

                onClicked: actionObject.trigger()
            }
        }
    }
}
