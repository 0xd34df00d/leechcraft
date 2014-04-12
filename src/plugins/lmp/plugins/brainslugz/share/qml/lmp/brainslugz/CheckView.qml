import QtQuick 1.1
import Effects 1.0

Rectangle {
    id: rootRect

    gradient: Gradient {
        GradientStop {
            position: 0
            color: colorProxy.color_TextView_TopColor
        }
        GradientStop {
            position: 1
            color: colorProxy.color_TextView_BottomColor
        }
    }

    state: checkingState

    states: [
        State {
            name: "checking"
            PropertyChanges {
                target: artistsToCheckView
                width: {
                    var cellsCount = rootRect.width / artistsToCheckView.cellWidth;
                    return Math.floor(cellsCount / 2) * artistsToCheckView.cellWidth;
                }
            }
        },
        State {
            name: "checked"
            PropertyChanges { target: artistsToCheckView; width: 0 }
        }
    ]

    GridView {
        id: artistsToCheckView

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: parent.width

        model: uncheckedModel

        cellWidth: 250
        cellHeight: 100

        delegate: Rectangle {
            id: gridDelegate

            radius: 5
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: colorProxy.color_TextBox_TopColor
                }
                GradientStop {
                    position: 1
                    color: colorProxy.color_TextBox_BottomColor
                }
            }

            border.width: 1
            border.color: colorProxy.color_TextBox_BorderColor
            smooth: true

            width: artistsToCheckView.cellWidth
            height: artistsToCheckView.cellHeight

            Text {
                text: artistName
                font.bold: true
                font.pointSize: 12
                color: colorProxy.color_TextBox_TitleTextColor
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 5
                width: parent.width
                elide: Text.ElideRight
            }

            GridView.onRemove: SequentialAnimation {
                PropertyAction { target: gridDelegate; property: "GridView.delayRemove"; value: true }
                NumberAnimation { target: gridDelegate; property: "width"; to: 0; duration: 500; easing.type: Easing.InCubic }
                PropertyAction { target: gridDelegate; property: "GridView.delayRemove"; value: false }
            }
        }
    }

    ListView {
        id: artistsView

        anchors.top: parent.top
        anchors.left: artistsToCheckView.right
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        model: checkedModel

        delegate: Rectangle {
            id: artistsDelegate

            radius: 5
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: colorProxy.color_TextBox_TopColor
                }
                GradientStop {
                    position: 1
                    color: colorProxy.color_TextBox_BottomColor
                }
            }

            border.width: 1
            border.color: colorProxy.color_TextBox_BorderColor
            smooth: true

            height: 2 * artistNameLabel.paintedHeight
            width: artistsView.width
            opacity: 0

            Text {
                id: artistNameLabel
                text: artistName
                font.bold: true
                font.pointSize: 12
                color: colorProxy.color_TextBox_TitleTextColor
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 5
                width: parent.width
                elide: Text.ElideRight
            }

            ListView.onAdd: ParallelAnimation {
                NumberAnimation { target: artistsDelegate; property: "opacity"; to: 1; duration: 500; easing.type: Easing.OutCubic }
                NumberAnimation { target: artistsDelegate; property: "width"; from:0; to: artistsView.width; duration: 500; easing.type: Easing.OutCubic }
            }
        }
    }
}

