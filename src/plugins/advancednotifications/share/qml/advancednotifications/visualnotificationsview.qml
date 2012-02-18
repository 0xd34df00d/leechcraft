import QtQuick 1.0
import "."

Rectangle {
    id: notifArea
    signal eventDismissed (string eventID)
    signal eventActionTriggered (string eventID, int index)

    width: 450; height: 200
    smooth: true
    radius: 16
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#CF414141" }
        GradientStop { position: 1.0; color: "#CF1A1A1A" }
    }

    Component {
        id: eventsDelegate
  
        Rectangle {
            id: eventRect
            property string eventID: object.eventID
 
            width: eventListView.width
            height: contentsRow.height + 5
            smooth: true
            radius: 9
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#DF3A3A3A" }
                GradientStop { position: 1.0; color: "#DF101010" }
            }

            Row {
                id: contentsRow

                height: Math.max(eventText.height, 32) + actionsListView.height
                anchors.fill: parent
                anchors.leftMargin: 2
                anchors.topMargin: 2

                Image {
                    id: eventPic
                    source: object.image

                    height: 32
                }

                Text {
                    id: eventText

                    text: object.extendedText
                    color: "lightgrey"
                }

                TextButton {
                    id: dismissButton

                    text: "x"
                    onClicked: { notifArea.eventDismissed (object.eventID) }
                }

                ListView {
                    id: actionsListView

                    height: 20
                    width: parent.width - 5
                    anchors.right: contentsRow.right
                    //anchors.leftMargin: 5
                    anchors.bottom: eventRect.bottom
                    anchors.bottomMargin: 5
                    anchors.top: eventPic.bottom
                    anchors.topMargin: 5

                    orientation: ListView.Horizontal

                    model: object.eventActionsModel
                    delegate: Rectangle {
                        height: actionsListView.height
                        width: actionText.width + 5
                        smooth: true
                        radius: 3
                        color: "transparent"

                        TextButton {
                            id: actionText

                            text: modelData
                            onClicked: { notifArea.eventActionTriggered (eventRect.eventID, index) }
                        }
                    }
 
                }
            }
        }
    }

    ListView {
        id: eventListView

        anchors.centerIn: parent
        anchors.topMargin: 5
        width: notifArea.width - 10
        height: notifArea.height

        model: eventsModel
        delegate: eventsDelegate
    }
}
