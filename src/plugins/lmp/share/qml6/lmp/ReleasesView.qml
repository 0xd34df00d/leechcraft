import QtQuick
import QtQuick.Effects
import "."

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

    Image {
        id: fullSizeAA
        state: "hidden"

        anchors.centerIn: parent
        width: parent.width - 60
        height: parent.height - 60
        z: 2
        smooth: true
        fillMode: Image.PreserveAspectFit

        visible: opacity != 0

        cache: false

        states: [
            State {
                name: "hidden"
                PropertyChanges { target: fullSizeAA; opacity: 0 }
                PropertyChanges { target: releasesViewBlur; blur: 0 }
            },
            State {
                name: "visible"
                PropertyChanges { target: fullSizeAA; opacity: 1 }
                PropertyChanges { target: releasesViewBlur; blur: 1 }
            }
        ]

        transitions: Transition {
            PropertyAnimation { property: "opacity"; duration: 300; easing.type: Easing.OutSine }
            PropertyAnimation { target: releasesViewBlur; property: "blur"; duration: 300; easing.type: Easing.OutSine }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: fullSizeAA.state = "hidden"
        }

        onStatusChanged: if (fullSizeAA.status == Image.Ready) fullSizeAA.state = "visible"
    }

    GridView {
        anchors.fill: parent
        id: releasesView

        visible: !releasesViewBlur.visible

        model: releasesModel
        cellHeight: 180
        cellWidth: 300

        delegate: Item {
            height: releasesView.cellHeight
            width: releasesView.cellWidth

            Rectangle {
                id: delegateRect
                anchors.fill: parent
                anchors.margins: 10
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

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true

                    onEntered: {
                        if (trackList.length > 0)
                        {
                            trackListContainer.text = trackList
                            trackListContainer.state = "visible"

                            var newPoint = delegateRect.mapToItem(rootRect, 0, 0)
                            trackListContainer.x = newPoint.x

                            var newY = newPoint.y + delegateRect.height
                            if (newY + trackListContainer.targetHeight >= rootRect.height)
                                newY = newPoint.y - trackListContainer.targetHeight
                            trackListContainer.y = newY
                        }
                        else
                            trackListContainer.state = ""
                    }
                    onExited: trackListContainer.state = ""
                }

                Column {
                    id: column1
                    anchors.fill: parent
                    anchors.topMargin: 5

                    Image {
                        id: albumImageThumb

                        width: 100
                        height: 100
                        smooth: true
                        fillMode: Image.PreserveAspectFit
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: albumThumbImage

                        cache: false

                        MouseArea {
                            anchors.fill: parent

                            onClicked: {
                                fullSizeAA.source = albumFullImage
                                if (fullSizeAA.status == Image.Ready) fullSizeAA.state = "visible"
                            }
                        }
                    }

                    Text {
                        id: albumNameLabel
                        text: albumName
                        horizontalAlignment: Text.AlignHCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                        elide: Text.ElideMiddle
                        font.bold: true
                        font.underline: true
                        font.pointSize: 12
                        color: colorProxy.color_TextBox_TitleTextColor

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: stdActions.openLink(releaseURL)
                        }
                    }

                    Text {
                        id: artistLabel
                        text: artistName
                        elide: Text.ElideMiddle
                        color: colorProxy.color_TextBox_TextColor
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pointSize: 8
                    }

                    Text {
                        id: dateLabel
                        text: releaseDate
                        color: colorProxy.color_TextBox_TextColor
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }
        }
    }

    MultiEffect {
        id: releasesViewBlur
        anchors.fill: releasesView
        source: releasesView

        blurEnabled: blur != 0
        visible: blurEnabled
    }

    TrackListContainer {
        id: trackListContainer
        width: releasesView.cellWidth - 20
    }
}
