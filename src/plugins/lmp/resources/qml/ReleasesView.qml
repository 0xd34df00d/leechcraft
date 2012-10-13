import QtQuick 1.0
import Effects 1.0

Rectangle {
    gradient: Gradient {
        GradientStop {
            position: 0
            color: "#53485F"
        }

        GradientStop {
            position: 1
            color: "#222222"
        }
    }
    anchors.fill: parent

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
                        position: 1
                        color: "#42394b"
                    }

                    GradientStop {
                        position: 0
                        color: "#000000"
                    }
                }

                border.width: 1
                border.color: "#000000"
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
                        font.pointSize: 12
                        color: "#dddddd"
                    }

                    Text {
                        id: artistLabel
                        text: artistName
                        elide: Text.ElideMiddle
                        color: "#999999"
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pointSize: 8
                    }

                    Text {
                        id: dateLabel
                        text: releaseDate
                        color: "#888888"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }
        }
    }
}
