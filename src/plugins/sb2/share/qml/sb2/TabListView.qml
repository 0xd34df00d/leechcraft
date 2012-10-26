import QtQuick 1.1

Rectangle {
    id: rootRect
    width: 600
    height: Math.min(600, tabsView.count * 36)
    smooth: true
    radius: 5
    focus: true

    signal closeRequested()
    signal tabSwitchRequested(int index)

    Keys.onEscapePressed: rootRect.closeRequested()

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

    ListView {
        id: tabsView
        anchors.fill: parent

        model: tabsListModel

        delegate: Item {
            width: tabsView.width
            height: 36

            Rectangle {
                id: tabRect
                anchors.fill: parent
                radius: 5
                smooth: true

                border.color: "black"
                border.width: 1

                Keys.onEscapePressed: rootRect.closeRequested()

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
                    id: tabIconImage
                    source: tabIcon

                    width: 32
                    height: 32
                    smooth: true

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                }

                Text {
                    id: tabNameLabel
                    text: tabName

                    color: "lightgrey"

                    anchors.left: tabIconImage.right
                    anchors.leftMargin: 4
                    anchors.verticalCenter: parent.verticalCenter
                }

                MouseArea {
                    id: rectMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onReleased: rootRect.tabSwitchRequested(index)
                }

                states: [
                    State {
                        name: "hovered"
                        when: rectMouseArea.containsMouse
                        PropertyChanges { target: tabRect; border.color: "#ff6500" }
                    }
                ]

                transitions: [
                    Transition {
                        from: ""
                        to: "hovered"
                        reversible: true
                        PropertyAnimation { properties: "border.color"; duration: 200 }
                    }
                ]
            }
        }
    }
}
