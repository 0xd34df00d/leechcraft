import QtQuick 1.0
import Effects 1.0
import org.LC.common 1.0
import "."

Rectangle {
    id: rootRect

    anchors.fill: parent
    smooth: true
    z: 0

    color: colorProxy.color_TextBox_TopColor

    signal bookmarkArtistRequested(string id, string page, string tags)
    signal previewRequested(string artist)
    signal linkActivated(string id)
    signal browseInfo(string artist)
    signal albumPreviewRequested(int idx)

    Image {
        id: fullSizeArtistImg
        state: "hidden"

        anchors.centerIn: parent
        width: parent.width - 64
        height: parent.height - 64
        z: 2
        smooth: true
        fillMode: Image.PreserveAspectFit

        visible: opacity != 0

        property bool navVisible: false

        states: [
            State {
                name: "hidden"
                when: fullSizeArtistImg.status == Image.Null || fullSizeArtistImg.status == Image.Error
                PropertyChanges { target: fullSizeArtistImg; opacity: 0 }
                PropertyChanges { target: bioViewBlur; blurRadius: 0 }
                PropertyChanges { target: loadProgress; opacity: 0 }
            },
            State {
                name: "loading"
                when: fullSizeArtistImg.status == Image.Loading
                PropertyChanges { target: bioViewBlur; blurRadius: 1 }
                PropertyChanges { target: loadProgress; opacity: 1 }
            },
            State {
                name: "visible"
                when: fullSizeArtistImg.status == Image.Ready
                PropertyChanges { target: fullSizeArtistImg; opacity: 1; width: parent.width - 64; height: parent.height - 64 }
                PropertyChanges { target: bioViewBlur; blurRadius: 10 }
                PropertyChanges { target: loadProgress; opacity: 0 }
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

        ActionButton {
            visible: parent.navVisible

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: 32
            height: width
            actionIconURL: "image://ThemeIcons/go-previous"

            onTriggered: artistImagesView.decrementCurrentIndex()
        }

        ActionButton {
            visible: parent.navVisible

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 32
            height: width
            actionIconURL: "image://ThemeIcons/go-next"

            onTriggered: artistImagesView.incrementCurrentIndex()
        }

        ProgressBar {
            id: loadProgress

            value: parent.progress * 100

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.bottom
            height: 12

            color: colorProxy.color_TextView_Aux3TextColor
        }
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

        StdArtistActions {
            id: artistActions
            z: 2
            anchors.left: artistNameLabel.right
            anchors.leftMargin: 8
            anchors.bottom: artistNameLabel.bottom

            visible: artistName.length > 0

            onBrowseInfo: rootRect.browseInfo(artistName)
            onBookmarkRequested: rootRect.bookmarkArtistRequested(artistName, artistPageURL, artistTags)
            onPreviewRequested: rootRect.previewRequested(artistName)
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
                    fullSizeArtistImg.navVisible = false
                    if (fullSizeArtistImg.status == Image.Ready)
                        fullSizeArtistImg.state = "visible"
                }
            }
        }

        Text {
            id: artistTagsLabel
            z: 2
            text: artistTags
            color: colorProxy.color_TextBox_Aux1TextColor
            anchors.left: artistActions.right
            anchors.leftMargin: 2
            anchors.bottom: artistNameLabel.bottom
            anchors.right: parent.right
            anchors.rightMargin: 2

            elide: Text.ElideRight
            horizontalAlignment: Text.AlignRight
            font.pointSize: 8
        }

        TrackListContainer {
            id: trackListContainer
            y: artistDiscoView.y
            x: artistDiscoView.x + artistDiscoView.width
        }

        ListView {
            id: artistDiscoView
            z: 2
            anchors.left: parent.left
            anchors.top: artistImageThumb.bottom
            anchors.topMargin: 2
            anchors.right: flickableBioText.left
            anchors.bottom: parent.bottom
            spacing: 5

            clip: true

            model: artistDiscoModel

            delegate: Item {
                width: artistDiscoView.width
                height: contentsRect.height

                Rectangle {
                    id: contentsRect
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: childrenRect.height + 5

                    color: "transparent"

                    Image {
                        id: albumArtImage
                        source: albumImage

                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: 20
                        anchors.rightMargin: 20

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
                        wrapMode: Text.Wrap
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
                        anchors.top: albumArtImage.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: albumYearLabel.bottom
                        hoverEnabled: true

                        onEntered: {
                            trackListContainer.text = albumTrackListTooltip
                            trackListContainer.state = "visible"
                            trackListContainer.y = Math.min(artistDiscoView.y + parent.parent.y - artistDiscoView.contentY,
                                    trackListContainer.parent.height - trackListContainer.targetHeight - 5)
                        }
                        onExited: trackListContainer.state = ""
                    }

                    PreviewAudioButton {
                        id: previewAudio

                        anchors.top: parent.top
                        anchors.topMargin: 2
                        anchors.right: parent.right
                        anchors.rightMargin: 2

                        onClicked: rootRect.albumPreviewRequested(index)
                    }
                }
            }
        }

        ListView {
            id: artistImagesView
            z: 3
            anchors.right: parent.right
            anchors.top: artistNameLabel.bottom
            anchors.bottom: parent.bottom
            width: Math.min(192, Math.max(rootRect.width / 8, 64))

            model: artistImagesModel

            clip: true

            spacing: 3

            keyNavigationWraps: true

            onCurrentItemChanged: currentItem.updateFullSize()

            delegate: Image {
                id: delegateItem
                source: thumbURL

                width: artistImagesView.width
                height: sourceSize.width > 0 ? Math.min(width, sourceSize.height * width / sourceSize.width) : width
                fillMode: Image.PreserveAspectFit

                smooth: true

                function updateFullSize() {
                    if (fullSizeArtistImg.state == "visible")
                        fullSizeArtistImg.source = fullURL
                }

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        fullSizeArtistImg.navVisible = true
                        fullSizeArtistImg.source = fullURL
                        if (fullSizeArtistImg.status == Image.Ready)
                            fullSizeArtistImg.state = "visible"

                        artistImagesView.currentIndex = index
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
            anchors.right: artistImagesView.left
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
