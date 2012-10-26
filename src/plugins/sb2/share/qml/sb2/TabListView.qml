import QtQuick 1.1

Rectangle {
    id: rootRect
    width: 600
    height: Math.min(600, tabsView.count * 40)
    smooth: true
    radius: 5

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

        delegate: Rectangle {
            id: tabRect
            width: tabsView.width
            height: 40
            radius: 5

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
        }
    }
}
