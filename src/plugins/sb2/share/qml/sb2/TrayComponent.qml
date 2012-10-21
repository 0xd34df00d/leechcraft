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

        delegate: Item {
            height: rootRect.trayItemHeight
            width: rootRect.width

            Rectangle {
                id: trayViewDelegate

                radius: width / 10
                smooth: true

                anchors.fill: parent
                anchors.margins: 2
                border.width: 1
                border.color: "black"

                states: [
                    State {
                        name: "hovered"
                        when: actionMouseArea.containsMouse
                        PropertyChanges { target: trayViewDelegate; border.color: "white"; anchors.margins: 0 }
                    }
                ]

                transitions: [
                    Transition {
                        PropertyAnimation { properties: "border.color"; duration: 200 }
                        AnchorAnimation { duration: 200 }
                    }
                ]

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
                    cache: false
                }

                MouseArea {
                    id: actionMouseArea

                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: actionObject.trigger()
                }
            }
        }
    }
}
