import QtQuick 1.0
import Effects 1.0

Rectangle {
    id: rootRect
    anchors.fill: parent

    signal linkActivated(string id)

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

        states: [
            State {
                name: "hidden"
                PropertyChanges { target: fullSizeAA; opacity: 0 }
                PropertyChanges { target: releasesViewBlur; blurRadius: 0 }
            },
            State {
                name: "visible"
                PropertyChanges { target: fullSizeAA; opacity: 1 }
                PropertyChanges { target: releasesViewBlur; blurRadius: 10 }
            }
        ]

        transitions: Transition {
            PropertyAnimation { property: "opacity"; duration: 300; easing.type: Easing.OutSine }
            PropertyAnimation { target: releasesViewBlur; property: "blurRadius"; duration: 300; easing.type: Easing.OutSine }
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

        model: releasesModel
        cellHeight: 180
        cellWidth: 300

        effect: Blur {
            id: releasesViewBlur
            blurRadius: 0.0
        }

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
                            onClicked: rootRect.linkActivated(releaseURL)
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
}
