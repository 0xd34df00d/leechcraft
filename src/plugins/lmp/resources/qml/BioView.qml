import QtQuick 1.0
import Effects 1.0

Rectangle {
    id: rootRect

    anchors.fill: parent
    smooth: true
    z: 0

    color: colorProxy.color_TextBox_TopColor

    signal linkActivated(string id)

    Image {
        id: fullSizeArtistImg
        state: "hidden"

        width: parent.width - 60
        height: parent.height - 60
        z: 2
        smooth: true
        fillMode: Image.PreserveAspectFit

        visible: opacity != 0

        states: [
            State {
                name: "hidden"
                PropertyChanges { target: fullSizeArtistImg; opacity: 0; width: artistImageThumb.width; height: artistImageThumb.height; x: artistImageThumb.x; y: artistImageThumb.y }
                PropertyChanges { target: bioViewBlur; blurRadius: 0 }
            },
            State {
                name: "visible"
                PropertyChanges { target: fullSizeArtistImg; opacity: 1; width: parent.width - 60; height: parent.height - 60; x: 30; y: 30 }
                PropertyChanges { target: bioViewBlur; blurRadius: 10 }
            }
        ]

        transitions: Transition {
            PropertyAnimation { properties: "opacity,width,height,x,y"; duration: 300; easing.type: Easing.OutSine }
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
            color: colorProxy.color_TextBox_TitleTextColor
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
            height: 170
            width: Math.min(height, sourceSize.width * height / sourceSize.height)
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
            color: colorProxy.color_TextBox_Aux1TextColor
            anchors.left: artistNameLabel.right
            anchors.leftMargin: 2
            anchors.bottom: artistNameLabel.bottom
            anchors.right: parent.right
            anchors.rightMargin: 2

            elide: Text.ElideRight
            horizontalAlignment: Text.AlignRight
            font.pointSize: 8
        }

        Rectangle {
            id: trackListContainer
            z: 0
            opacity: 0
            y: artistDiscoView.y

            radius: 5
            width: 400
            height: trackListText.height + 10
            clip: true

            color: colorProxy.setAlpha(colorProxy.color_TextBox_TopColor, 0.9)

            border.color: colorProxy.color_TextBox_HighlightBorderColor
            border.width: 1

            Text {
                id: trackListText
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 5

                elide: Text.ElideRight
                color: colorProxy.color_TextBox_TextColor
            }

            states: [
                State {
                    name: "visible"
                    PropertyChanges { target: trackListContainer; z: 5; opacity: 1 }
                }
            ]

            Behavior on y { PropertyAnimation { duration: 200 } }
            Behavior on height { PropertyAnimation { duration: 200 } }

            transitions: Transition {
                ParallelAnimation {
                    PropertyAnimation { property: "opacity"; duration: 300; easing.type: Easing.OutSine }
                }
            }
        }

        ListView {
            id: artistDiscoView
            z: 2
            anchors.left: parent.left
            anchors.top: artistImageThumb.bottom
            anchors.topMargin: 2
            anchors.right: flickableBioText.left
            anchors.bottom: parent.bottom

            clip: true

            model: artistDiscoModel

            delegate: Item {
                width: artistDiscoView.width
                height: artistDiscoView.width

                Rectangle {
                    anchors.fill: parent

                    color: "transparent"

                    Image {
                        id: albumArtImage
                        source: albumImage

                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: 20
                        anchors.rightMargin: 20
                        height: width

                        smooth: true
                        fillMode: Image.PreserveAspectFit
                    }

                    Text {
                        id: albumNameLabel
                        anchors.top: albumArtImage.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right

                        text: albumName
                        color: colorProxy.color_TextBox_TextColor
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        id: albumYearLabel
                        anchors.top: albumNameLabel.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right

                        text: albumYear
                        color: colorProxy.color_TextBox_Aux2TextColor
                        horizontalAlignment: Text.AlignHCenter
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true

                        onEntered: {
                            trackListText.text = albumTrackListTooltip
                            trackListContainer.state = "visible"
                            trackListContainer.x = artistDiscoView.x + artistDiscoView.width
                            trackListContainer.y = Math.min(artistDiscoView.y + parent.parent.y - artistDiscoView.contentY,
                                    trackListContainer.parent.height - trackListContainer.height - 5)
                        }
                        onExited: trackListContainer.state = ""
                    }
                }
            }
        }

        Rectangle {
            id: upTextShade
            z: 3
            height: 10

            anchors.top: artistNameLabel.bottom
            anchors.left: artistImageThumb.right
            anchors.right: parent.right

            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: colorProxy.color_TextBox_TopColor
                }
                GradientStop {
                    position: 1
                    color: colorProxy.setAlpha(colorProxy.color_TextBox_TopColor, 0)
                }
            }
        }

        Flickable {
            z: 2

            id: flickableBioText

            anchors.leftMargin: 5
            anchors.left: artistImageThumb.right
            anchors.rightMargin: 5
            anchors.right: parent.right
            anchors.top: artistNameLabel.bottom
            anchors.bottom: parent.bottom

            contentWidth: width
            contentHeight: shortDescLabel.height + 16

            clip: true

            Text {
                id: shortDescLabel
                text: artistInfo
                textFormat: Text.RichText
                clip: true
                color: colorProxy.color_TextBox_TextColor
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
                    color: colorProxy.color_TextBox_TopColor
                }
                GradientStop {
                    position: 1
                    color: colorProxy.color_TextBox_BottomColor
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
                    color: colorProxy.setAlpha(colorProxy.color_TextBox_BottomColor, 0)
                }

                GradientStop {
                    position: 1
                    color: colorProxy.color_TextBox_BottomColor
                }
            }
        }
    }
}
