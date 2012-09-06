import QtQuick 1.0
import Effects 1.0

Rectangle {
    id: rootRect

    anchors.fill: parent
    smooth: true
    z: 0

    color: "black"

    signal linkActivated(string id)

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
                PropertyChanges { target: bioViewBlur; blurRadius: 0 }
            },
            State {
                name: "visible"
                PropertyChanges { target: fullSizeArtistImg; opacity: 1 }
                PropertyChanges { target: bioViewBlur; blurRadius: 10 }
            }
        ]

        transitions: Transition {
            PropertyAnimation { property: "opacity"; duration: 300; easing.type: Easing.OutSine }
            PropertyAnimation { target: bioViewBlur; property: "blurRadius"; duration: 300; easing.type: Easing.OutSine }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: fullSizeArtistImg.state = "hidden"
        }

        onStatusChanged: if (fullSizeArtistImg.status == Image.Ready) fullSizeArtistImg.state = "visible"
    }

    Rectangle {
        anchors.fill: parent
        color: "#00000000"

        effect: Blur {
            id: bioViewBlur
            blurRadius: 0.0
        }

        Text {
            id: artistNameLabel
            z: 2
            text: artistName
            font.bold: true
            font.underline: true
            font.pointSize: 12
            color: "#dddddd"
            anchors.top: parent.top
            anchors.topMargin: 2
            anchors.left: artistImageThumb.right
            anchors.leftMargin: 5

            MouseArea {
                anchors.fill: parent

                onClicked: rootRect.linkActivated(artistPageURL)
            }
        }

        Image {
            id: artistImageThumb
            z: 2
            width: 170
            height: 170
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
                    if (fullSizeArtistImg.status == Image.Ready) fullSizeArtistImg.state = "visible"
                }
            }
        }

        Text {
            id: artistTagsLabel
            z: 2
            text: artistTags
            color: "#999999"
            anchors.leftMargin: 5
            anchors.left: artistImageThumb.right
            anchors.top: artistNameLabel.bottom
            anchors.topMargin: 0
            font.pointSize: 8
        }

        Rectangle {
            id: upTextShade
            z: 3
            height: 10

            anchors.top: artistImageThumb.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            gradient: Gradient {
                GradientStop {
                    position: 1
                    color: "#00000000"
                }

                GradientStop {
                    position: 0
                    color: "#ff000000"
                }
            }
        }

        Flickable {
            z: 2

            anchors.leftMargin: 5
            anchors.left: parent.left
            anchors.rightMargin: 5
            anchors.right: parent.right
            anchors.top: artistImageThumb.bottom
            anchors.bottom: parent.bottom

            contentWidth: width
            contentHeight: shortDescLabel.height + 16

            clip: true

            Text {
                id: shortDescLabel
                text: artistInfo
                textFormat: Text.RichText
                clip: true
                color: "#aaaaaa"
                wrapMode: Text.WordWrap

                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.left: parent.left
                anchors.right: parent.right

                onLinkActivated: rootRect.linkActivated(link)
            }
        }

        Rectangle {
            z: 1
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: artistImageThumb.bottom
            anchors.bottom: parent.bottom

            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#000000"
                }

                GradientStop {
                    position: 1
                    color: "#42394b"
                }
            }
        }

        Rectangle {
            id: downTextShade
            z: 3
            height: 10

            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#0042394b"
                }

                GradientStop {
                    position: 1
                    color: "#ff42394b"
                }
            }
        }
    }
}
