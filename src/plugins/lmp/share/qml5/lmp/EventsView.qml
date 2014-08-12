import QtQuick 2.3
import QtGraphicalEffects 1.0
import "."

Rectangle {
    id: rootRect

    signal attendSure(int id)
    signal attendMaybe(int id)
    signal unattend(int id)

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

        cache: false

        states: [
            State {
                name: "hidden"
                PropertyChanges { target: fullSizeEventImg; opacity: 0 }
                PropertyChanges { target: eventsViewBlur; radius: 0 }
            },
            State {
                name: "visible"
                PropertyChanges { target: fullSizeEventImg; opacity: 1 }
                PropertyChanges { target: eventsViewBlur; radius: 20 }
            }
        ]

        transitions: Transition {
            PropertyAnimation { property: "opacity"; duration: 300; easing.type: Easing.OutSine }
            PropertyAnimation { target: eventsViewBlur; property: "radius"; duration: 300; easing.type: Easing.OutSine }
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

        visible: eventsViewBlur.radius == 0

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

                Rectangle {
                    id: fillRect
                    radius: parent.radius
                    anchors.fill: parent
                    color: "#aa000000"
                    visible: isAttended
                    z: 5
                }

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
                    cache: false

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
                    color: colorProxy.color_TextBox_TitleTextColor
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    anchors.left: eventImageThumb.right
                    anchors.leftMargin: 5
                    anchors.right: parent.right
                }

                Text {
                    id: eventDateLabel
                    text: eventDate
                    color: colorProxy.color_TextBox_TextColor
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
                    color: colorProxy.color_TextBox_TextColor
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
                    color: colorProxy.color_TextBox_Aux1TextColor
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
                    color: colorProxy.color_TextBox_TitleTextColor
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
                    color: colorProxy.color_TextBox_Aux1TextColor
                    clip: true
                    anchors.left: eventImageThumb.right
                    anchors.leftMargin: 5
                    anchors.top: eventHeadlinerLabel.bottom
                    anchors.topMargin: 0
                    anchors.right: parent.right
                    font.pointSize: 8
                }

                TextButton {
                    id: attendMaybe
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    anchors.right: parent.right
                    anchors.rightMargin: 2
                    visible: !isAttended

                    text: attendSureTextString

                    onClicked: rootRect.attendSure(eventID)
                }

                TextButton {
                    id: attendSure
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    anchors.right: attendMaybe.left
                    anchors.rightMargin: 2
                    visible: !isAttended

                    text: attendMaybeTextString

                    onClicked: rootRect.unattendMaybe(eventID)
                }

                TextButton {
                    id: unAttend
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    anchors.right: parent.right
                    anchors.rightMargin: 2
                    visible: isAttended

                    text: unattendTextString

                    onClicked: rootRect.unattend(eventID)
                }
            }
        }
    }

    GaussianBlur {
        id: eventsViewBlur
        anchors.fill: eventsView

        radius: 0
        source: eventsView
        samples: 32

        visible: radius != 0
    }
}
