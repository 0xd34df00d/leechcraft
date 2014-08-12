import QtQuick 2.3
import QtGraphicalEffects 1.0
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
    anchors.fill: parent

    signal bookmarkArtistRequested(string id, string page, string tags)
    signal previewRequested(string artist)
    signal linkActivated(string id)
    signal browseInfo(string artist)

    property alias model: similarView.model

    Image {
        id: fullSizeArtistImg
        state: "hidden"

        anchors.centerIn: parent
        width: parent.width - 60
        height: parent.height - 60
        z: 2
        smooth: true
        fillMode: Image.PreserveAspectFit
        cache: false

        visible: opacity != 0

        states: [
            State {
                name: "hidden"
                PropertyChanges { target: fullSizeArtistImg; opacity: 0 }
                PropertyChanges { target: similarViewBlur; radius: 0 }
            },
            State {
                name: "visible"
                PropertyChanges { target: fullSizeArtistImg; opacity: 1 }
                PropertyChanges { target: similarViewBlur; radius: 20 }
            }
        ]

        transitions: Transition {
            ParallelAnimation {
                PropertyAnimation { property: "opacity"; duration: 300; easing.type: Easing.OutSine }
                PropertyAnimation { target: similarViewBlur; property: "radius"; duration: 300; easing.type: Easing.OutSine }
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: fullSizeArtistImg.state = "hidden"
        }

        onStatusChanged: if (fullSizeArtistImg.status == Image.Ready) fullSizeArtistImg.state = "visible"
    }

    ListView {
        anchors.fill: parent
        id: similarView
        smooth: true

        visible: similarViewBlur.radius == 0

        model: similarModel
        delegate: Item {
            height: 150
            width: similarView.width
            smooth: true

            Rectangle {
                id: delegateRect

                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                anchors.topMargin: 5
                anchors.bottomMargin: 5

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

                Text {
                    id: artistNameLabel
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
                    width: 100
                    height: 100
                    smooth: true
                    fillMode: Image.PreserveAspectFit
                    anchors.left: parent.left
                    anchors.leftMargin: 2
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    source: artistImageURL
                    cache: false

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            fullSizeArtistImg.source = artistBigImageURL
                            if (fullSizeArtistImg.status == Image.Ready) fullSizeArtistImg.state = "visible"
                        }
                    }
                }

                StdArtistActions {
                    id: artistActions

                    anchors.bottom: artistNameLabel.bottom
                    anchors.left: artistNameLabel.right
                    anchors.leftMargin: 8

                    previewVisible: !artistInCollection
                    bookmarkVisible: !artistInCollection

                    onBrowseInfo: rootRect.browseInfo(artistName)
                    onBookmarkRequested: rootRect.bookmarkArtistRequested(artistName, artistPageURL, artistTags)
                    onPreviewRequested: rootRect.previewRequested(artistName)
                }

                Text {
                    id: similarityLabel
                    text: similarity
                    color: colorProxy.color_TextBox_Aux1TextColor
                    anchors.top: artistNameLabel.bottom
                    anchors.topMargin: 2
                    anchors.left: artistImageThumb.right
                    anchors.leftMargin: 5
                }

                Text {
                    id: artistTagsLabel
                    text: artistTags
                    color: colorProxy.color_TextBox_Aux2TextColor
                    anchors.leftMargin: 5
                    anchors.left: artistImageThumb.right
                    anchors.top: similarityLabel.bottom
                    anchors.topMargin: 0
                    anchors.right: parent.right
                    anchors.rightMargin: 5
                    elide: Text.ElideRight
                    font.pointSize: 8
                }

                Text {
                    id: shortDescLabel
                    text: shortDesc
                    textFormat: Text.RichText
                    width: parent.width - artistImageThumb.width - 10
                    clip: true
                    color: colorProxy.color_TextBox_TextColor
                    wrapMode: Text.WordWrap
                    anchors.leftMargin: 5
                    anchors.left: artistImageThumb.right
                    anchors.top: artistTagsLabel.bottom
                    anchors.topMargin: 5
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 1

                    onLinkActivated: rootRect.linkActivated(link)
                }

                Rectangle {
                    id: downTextShade
                    z: 3
                    height: 10
                    radius: parent.radius

                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 1
                    anchors.left: parent.left
                    anchors.leftMargin: 1
                    anchors.right: parent.right
                    anchors.rightMargin: 1

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
    }

    GaussianBlur {
        id: similarViewBlur
        anchors.fill: similarView

        radius: 0
        source: similarView
        samples: 32

        visible: radius != 0
    }
}
