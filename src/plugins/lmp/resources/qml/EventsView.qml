import QtQuick 1.0

Rectangle {
    id: rootRect

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
/*
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

        onStatusChanged: if (fullSizeArtistImg.status == Image.Ready) fullSizeArtistImg.state = "visible"
    }
*/
    ListView {
        anchors.fill: parent
        id: eventsView

        model: eventsModel
        delegate: Item {
            height: 110
            width: eventsView.width

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

                Image {
                    id: eventImageThumb
                    width: 100
                    height: 100
                    smooth: true
                    fillMode: Image.PreserveAspectFit
                    anchors.left: parent.left
                    anchors.leftMargin: 2
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    source: eventImageThumbURL
                }

                Text {
                    id: eventNameLabel
                    text: eventName
                    font.bold: true
                    font.pointSize: 12
                    color: "#dddddd"
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    anchors.left: eventImageThumb.right
                    anchors.leftMargin: 5
                }

                Text {
                    id: eventDateLabel
                    text: eventDate
                    color: "#bbbbbb"
                    anchors.left: eventImageThumb.right
                    anchors.leftMargin: 5
                    anchors.top: eventNameLabel.bottom
                    anchors.topMargin: 2
                    font.pointSize: 10
                }

                Text {
                    id: eventPlaceLabel
                    text: eventCity.length ? eventCity + ", " + eventPlace : eventPlace
                    color: "#aaaaaa"
                    anchors.left: eventImageThumb.right
                    anchors.leftMargin: 5
                    anchors.top: eventDateLabel.bottom
                    anchors.topMargin: 0
                    font.pointSize: 9
                }

                Text {
                    id: eventTagsLabel
                    text: eventTags
                    color: "#999999"
                    anchors.left: eventImageThumb.right
                    anchors.leftMargin: 5
                    anchors.top: eventPlaceLabel.bottom
                    anchors.topMargin: 0
                    font.pointSize: 8
                }

                Text {
                    id: eventHeadlinerLabel
                    text: eventArtists.length ? eventHeadliner : ""
                    font.bold: true
                    color: "#dddddd"
                    anchors.left: eventImageThumb.right
                    anchors.leftMargin: 5
                    anchors.top: eventTagsLabel.bottom
                    anchors.topMargin: 5
                    font.pointSize: 8
                }

                Text {
                    id: eventArtistsLabel
                    text: eventArtists
                    color: "#999999"
                    anchors.left: eventImageThumb.right
                    anchors.leftMargin: 5
                    anchors.top: eventHeadlinerLabel.bottom
                    anchors.topMargin: 0
                    font.pointSize: 8
                }

                /*
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
                            if (fullSizeArtistImg.status == Image.Ready) fullSizeArtistImg.state = "visible"
                        }
                    }
                }

                Image {
                    id: addToList

                    width: 16
                    height: 16
                    smooth: true
                    fillMode: Image.PreserveAspectFit

                    anchors.top: parent.top
                    anchors.topMargin: 2
                    anchors.left: artistNameLabel.right
                    anchors.leftMargin: 8
                    source: "image://sysIcons/bookmark-new"
                    visible: !artistInCollection

                    MouseArea {
                        id: addToListArea
                        anchors.fill: parent
                        anchors.margins: -2
                        hoverEnabled: true

                        onClicked: {
                            rootRect.bookmarkArtistRequested(artistName, artistPageURL, artistTags)
                        }
                    }

                    Rectangle {
                        id: addToListHover
                        anchors.fill: parent
                        anchors.margins: -1
                        radius: 2

                        visible: addToListArea.containsMouse

                        color: "#00000000"
                        border.width: 1
                        border.color: "#888888"
                    }
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
                */
            }
        }
    }
}
