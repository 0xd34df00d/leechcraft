import QtQuick 2.3
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import org.LC.common 1.0
import "."

Rectangle {
    id: rootRect

    smooth: true

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

        cache: false

        property bool navVisible: false

        states: [
            State {
                name: "hidden"
                when: fullSizeArtistImg.status == Image.Null || fullSizeArtistImg.status == Image.Error
                PropertyChanges { target: fullSizeArtistImg; opacity: 0 }
                PropertyChanges { target: bioViewBlur; radius: 0 }
                PropertyChanges { target: loadProgress; opacity: 0 }
            },
            State {
                name: "loading"
                when: fullSizeArtistImg.status == Image.Loading
                PropertyChanges { target: bioViewBlur; radius: 2 }
                PropertyChanges { target: loadProgress; opacity: 1 }
            },
            State {
                name: "visible"
                when: fullSizeArtistImg.status == Image.Ready
                PropertyChanges { target: fullSizeArtistImg; opacity: 1; width: parent.width - 64; height: parent.height - 64 }
                PropertyChanges { target: bioViewBlur; radius: 20 }
                PropertyChanges { target: loadProgress; opacity: 0 }
            }
        ]

        transitions: Transition {
            PropertyAnimation { properties: "opacity,width,height,x,y"; duration: 300; easing.type: Easing.OutSine }
            PropertyAnimation { target: bioViewBlur; property: "radius"; duration: 300; easing.type: Easing.OutSine }
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

            onTriggered: artistImagesView.moveCurrentIndexLeft()
        }

        ActionButton {
            visible: parent.navVisible

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 32
            height: width
            actionIconURL: "image://ThemeIcons/go-next"

            onTriggered: artistImagesView.moveCurrentIndexRight()
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

    Item {
        id: artistArea

        anchors.fill: parent

        visible: bioViewBlur.radius == 0

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
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

        Item {
            id: leftPane

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: artistImageThumb.width

            z: 1

            Image {
                id: artistImageThumb
                height: 170
                width: Math.min(height, sourceSize.width * height / sourceSize.height)
                smooth: true
                fillMode: Image.PreserveAspectFit
                anchors.left: parent.left
                anchors.top: parent.top
                source: artistImageURL
                cache: false

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

            ListView {
                id: artistDiscoView
                anchors.left: parent.left
                anchors.top: artistImageThumb.bottom
                anchors.topMargin: 2
                anchors.right: artistImageThumb.right
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

                            asynchronous: false
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

            TrackListContainer {
                id: trackListContainer
                y: artistDiscoView.y
                x: artistDiscoView.x + artistDiscoView.width
            }
        }

        Item {
            id: rightPane

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: leftPane.right
            anchors.right: parent.right

            Text {
                id: artistNameLabel
                text: artistName
                font.bold: true
                font.underline: true
                font.pointSize: 12
                color: colorProxy.color_TextBox_TitleTextColor
                anchors.top: parent.top
                anchors.topMargin: 2
                anchors.left: parent.left
                anchors.leftMargin: 5

                MouseArea {
                    anchors.fill: parent

                    onClicked: rootRect.linkActivated(artistPageURL)
                }
            }

            StdArtistActions {
                id: artistActions
                anchors.left: artistNameLabel.right
                anchors.leftMargin: 8
                anchors.bottom: artistNameLabel.bottom

                visible: artistName.length > 0

                onBrowseInfo: rootRect.browseInfo(artistName)
                onBookmarkRequested: rootRect.bookmarkArtistRequested(artistName, artistPageURL, artistTags)
                onPreviewRequested: rootRect.previewRequested(artistName)
            }

            Text {
                id: artistTagsLabel
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

            ColumnLayout {
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.right: parent.right
                anchors.rightMargin: 5
                anchors.top: artistNameLabel.bottom
                anchors.bottom: parent.bottom

                ScrollView {
                    id: flickableBioText

                    style: LMPScrollStyle {}

                    Layout.preferredHeight: flickableBioTextFlickable.contentHeight
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft

                    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

                    Flickable {
                        id: flickableBioTextFlickable

                        contentWidth: width
                        contentHeight: shortDescLabel.implicitHeight

                        clip: true

                        Text {
                            id: shortDescLabel
                            text: artistInfo
                            textFormat: Text.RichText
                            clip: true
                            color: colorProxy.color_TextBox_TextColor
                            wrapMode: Text.WordWrap

                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right

                            onLinkActivated: rootRect.linkActivated(link)
                        }
                    }
                }

                GridView {
                    id: artistImagesView

                    Layout.preferredHeight: 128 * Math.ceil(count / Math.floor(parent.width / 128))
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft

                    model: artistImagesModel

                    cellWidth: cellHeight
                    cellHeight: Math.min(128, Math.max(height / 3, 64))
                    clip: true

                    keyNavigationWraps: true

                    onCurrentItemChanged: currentItem.updateFullSize()

                    delegate: Image {
                        id: delegateItem
                        source: thumbURL

                        height: artistImagesView.cellHeight
                        width: sourceSize.height > 0 ? Math.min(height, sourceSize.width * height / sourceSize.height) : height
                        fillMode: Image.PreserveAspectFit

                        cache: false

                        smooth: true
                        opacity: 0

                        function updateFullSize() {
                            if (fullSizeArtistImg.state == "visible")
                                fullSizeArtistImg.source = fullURL
                        }

                        states: State {
                            name: "ready"
                            when: delegateItem.status == Image.Ready
                            PropertyChanges { target: delegateItem; opacity: 1 }
                        }

                        transitions: Transition {
                            from: "*"
                            to: "ready"
                            NumberAnimation { target: delegateItem; properties: "opacity"; duration: 400 }
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
            }
        }
    }

    GaussianBlur {
        id: bioViewBlur

        anchors.fill: artistArea

        radius: 0
        source: artistArea
        samples: 32

        visible: radius != 0
    }
}
