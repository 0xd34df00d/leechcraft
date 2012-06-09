import QtQuick 1.0

Rectangle {
    gradient: Gradient {
        GradientStop {
            position: 0
            color: "#ff6600"
        }

        GradientStop {
            position: 1
            color: "#ff1d00"
        }
    }
    anchors.fill: parent

    Image {
        id: fullSizeArtistImg
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
                PropertyChanges { target: fullSizeArtistImg; opacity: 0 }
            },
            State {
                name: "visible"
                PropertyChanges { target: fullSizeArtistImg; opacity: 1 }
            }
        ]

        transitions: Transition {
            PropertyAnimation { property: "opacity"; duration: 300; easing.type: Easing.OutSine }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: fullSizeArtistImg.state = "hidden"
        }
    }

    ListView {
        anchors.fill: parent
        id: similarView

        model: similarModel
        spacing: 5
        delegate: Rectangle {
            id: delegateRect
            height: 150
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

            border.width: 0
            border.color: "#000000"
            smooth: true
            width: similarView.width
            Text {
                id: artistNameLabel
                text: artistName
                font.bold: true
                font.pointSize: 12
                color: "#dddddd"
                anchors.top: parent.top
                anchors.topMargin: 2
                anchors.left: artistImageThumb.right
                anchors.leftMargin: 5
            }

            Image {
                id: artistImageThumb
                width: 100
                height: 100
                smooth: true
                fillMode: Image.PreserveAspectFit
                anchors.left: parent.left
                anchors.leftMargin: 2
                anchors.top: parent.top
                anchors.topMargin: 2
                source: artistImageURL

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        fullSizeArtistImg.source = artistBigImageURL
                        fullSizeArtistImg.state = "visible"
                    }
                }
            }

            Text {
                id: similarityLabel
                text: similarity
                color: "#888888"
                anchors.top: parent.top
                anchors.topMargin: 2
                anchors.right: parent.right
                anchors.rightMargin: 2
            }

            Text {
                id: artistTagsLabel
                text: artistTags
                color: "#999999"
                anchors.leftMargin: 5
                anchors.left: artistImageThumb.right
                anchors.top: artistNameLabel.bottom
                anchors.topMargin: 0
                font.pointSize: 8
            }

            Text {
                id: shortDescLabel
                text: shortDesc
                textFormat: Text.RichText
                width: parent.width - artistImageThumb.width - 10
                clip: true
                color: "#aaaaaa"
                wrapMode: Text.WordWrap
                anchors.leftMargin: 5
                anchors.left: artistImageThumb.right
                anchors.top: artistTagsLabel.bottom
                anchors.topMargin: 5
                anchors.bottom: parent.bottom
            }
        }
    }
}
