import QtQuick 1.0
import Effects 1.0

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

    Image {
        id: fullSizeEventImg
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
                PropertyChanges { target: fullSizeEventImg; opacity: 0 }
                PropertyChanges { target: eventsViewBlur; blurRadius: 0 }
            },
            State {
                name: "visible"
                PropertyChanges { target: fullSizeEventImg; opacity: 1 }
                PropertyChanges { target: eventsViewBlur; blurRadius: 10 }
            }
        ]

        transitions: Transition {
            PropertyAnimation { property: "opacity"; duration: 300; easing.type: Easing.OutSine }
            PropertyAnimation { target: eventsViewBlur; property: "blurRadius"; duration: 300; easing.type: Easing.OutSine }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: fullSizeEventImg.state = "hidden"
        }

        onStatusChanged: if (fullSizeEventImg.status == Image.Ready) fullSizeEventImg.state = "visible"
    }

    ListView {
        anchors.fill: parent
        id: eventsView

        effect: Blur {
            id: eventsViewBlur
            blurRadius: 0.0
        }

        model: eventsModel
        delegate: Item {
            height: 115
            width: eventsView.width
            clip: true

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

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            fullSizeEventImg.source = eventImageBigURL
                            if (fullSizeEventImg.status == Image.Ready) fullSizeEventImg.state = "visible"
                        }
                    }
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
                    anchors.right: parent.right
                }

                Text {
                    id: eventDateLabel
                    text: eventDate
                    color: "#bbbbbb"
                    anchors.left: eventImageThumb.right
                    anchors.leftMargin: 5
                    anchors.top: eventNameLabel.bottom
                    anchors.topMargin: 2
                    anchors.right: parent.right
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
                    anchors.right: parent.right
                    font.pointSize: 9
                }

                Text {
                    id: eventTagsLabel
                    text: eventTags
                    color: "#999999"
                    clip: true
                    anchors.left: eventImageThumb.right
                    anchors.leftMargin: 5
                    anchors.top: eventPlaceLabel.bottom
                    anchors.topMargin: 0
                    anchors.right: parent.right
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
                    anchors.right: parent.right
                    font.pointSize: 8
                }

                Text {
                    id: eventArtistsLabel
                    text: eventArtists
                    color: "#999999"
                    clip: true
                    anchors.left: eventImageThumb.right
                    anchors.leftMargin: 5
                    anchors.top: eventHeadlinerLabel.bottom
                    anchors.topMargin: 0
                    anchors.right: parent.right
                    font.pointSize: 8
                }
            }
        }
    }
}
