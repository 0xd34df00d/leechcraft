import QtQuick 2.3
import org.LC.common 1.0
import ".."

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

        model: artistsModel

        cellWidth: 250
        cellHeight: 250

        delegate: Rectangle {
            id: gridDelegate

            radius: 5
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: Qt.tint(colorProxy.color_TextBox_TopColor, gridDelegate.tintColor)
                }
                GradientStop {
                    position: 1
                    color: Qt.tint(colorProxy.color_TextBox_BottomColor, gridDelegate.tintColor)
                }
            }

            property color tintColor: !isChecked ? "#00000000" :
                            (missingCount > presentCount ? "#20ff0000" :
                                    (missingCount ? "#20ffff00" : "#2000ff00"))
            Behavior on tintColor { PropertyAnimation { duration: 500 } }

            border.width: 1
            border.color: colorProxy.color_TextBox_BorderColor
            smooth: true

            width: artistsToCheckView.cellWidth
            height: artistsToCheckView.cellHeight

            Image {
                id: artistImage
                source: artistImageUrl

                anchors.top: parent.top
                anchors.topMargin: 10
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 30
                anchors.rightMargin: 30
                height: width

                smooth: true
                fillMode: Image.PreserveAspectFit
            }

            Text {
                text: artistName
                font.bold: true
                font.pointSize: 12
                color: colorProxy.color_TextBox_TitleTextColor
                anchors.top: artistImage.bottom
                anchors.topMargin: 10
                anchors.left: parent.left
                anchors.right: parent.right
                width: parent.width
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
            }

            ActionButton {
                id: toggleButton
                visible: rootRect.state == ""

                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.right: parent.right
                anchors.rightMargin: 8
                width: 32
                height: width

                actionIconURL: scheduled ? "image://ThemeIcons/list-remove" : "image://ThemeIcons/list-add"
                onTriggered: artistsModel.setArtistScheduled(artistId, !scheduled)
            }

            GridView.onRemove: SequentialAnimation {
                PropertyAction { target: gridDelegate; property: "GridView.delayRemove"; value: true }
                NumberAnimation { target: gridDelegate; property: "scale"; to: 0; duration: 500; easing.type: Easing.InCubic }
                PropertyAction { target: gridDelegate; property: "GridView.delayRemove"; value: false }
            }
        }
    }

    ListView {
        id: artistsView

        anchors.top: parent.top
        anchors.left: artistsToCheckView.right
        anchors.leftMargin: 10
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
                return artistNameLabel.height + rows * releasesView.cellHeight + releasesView.anchors.topMargin;
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
                width: parent.width
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
            }

            GridView {
                id: releasesView
                anchors.top: artistNameLabel.bottom
                anchors.topMargin: 10
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                interactive: false

                model: releases

                cellWidth: 190
                cellHeight: 190

                delegate: Rectangle {
                    width: releasesView.cellWidth
                    height: releasesView.cellHeight
                    color: "transparent"

                    Image {
                        id: releaseArtImage
                        source: releaseArt

                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: 30
                        anchors.rightMargin: 30
                        height: width

                        smooth: true
                        fillMode: Image.PreserveAspectFit

                        cache: false
                    }

                    Text {
                        id: releaseNameLabel
                        anchors.top: releaseArtImage.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right

                        text: releaseName
                        color: colorProxy.color_TextBox_TextColor
                        horizontalAlignment: Text.AlignHCenter

                        wrapMode: Text.Wrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
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

                    PreviewAudioButton {
                        id: previewAudio
                        anchors.top: parent.top
                        anchors.right: parent.right
                        onClicked: artistsModel.previewRelease(artistId, index)
                    }
                }
            }
        }
    }
}

