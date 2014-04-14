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

            height: {
                if (!releasesView.width)
                    return artistNameLabel.height;
                var releasesInRow = Math.floor(releasesView.width / releasesView.cellWidth);
                var rows = Math.ceil(releasesView.count / releasesInRow);
                return artistNameLabel.height + rows * releasesView.cellHeight;
            }
            width: artistsView.width

            Text {
                id: artistNameLabel
                text: artistName
                font.bold: true
                font.pointSize: 12
                color: colorProxy.color_TextBox_TitleTextColor
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: 5
                width: parent.width
                elide: Text.ElideRight
            }

            GridView {
                id: releasesView
                anchors.top: artistNameLabel.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                interactive: false

                model: releases

                cellWidth: 150
                cellHeight: 150

                delegate: Rectangle {
                    width: releasesView.cellWidth
                    height: releasesView.cellHeight
                    color: "transparent"

                    Text {
                        id: releaseNameLabel
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right

                        text: releaseName
                        color: colorProxy.color_TextBox_TextColor
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.Wrap
                    }

                    Text {
                        id: releaseYearLabel
                        anchors.top: releaseNameLabel.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right

                        text: releaseYear
                        color: colorProxy.color_TextBox_Aux2TextColor
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
    }
}

