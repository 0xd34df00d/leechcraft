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

    ListView {
        id: eventListView

        anchors.fill: parent
        anchors { topMargin: 5; leftMargin: 5; rightMargin: 5 }
        
        model: eventsModel
        delegate: Rectangle {
            id: eventRect
            property string eventID: object.eventID

            height: eventInfoRow.height + actionsArea.height + 5
            width: eventListView.width

            smooth: true
            radius: 9

            gradient: Gradient {
                GradientStop { position: 0.0; color: "#DF3A3A3A" }
                GradientStop { position: 1.0; color: "#DF101010" }
            }

            Item {
                id: eventInfoRow
                anchors { left: parent.left; right: parent.right; top: parent.top }
                height: Math.max (eventPic.height, eventText.height, dismissButton.height)
    
                Image {
                    id: eventPic
                    anchors { left: parent.left; top: parent.top }
                
                    source: object.image
                    height: 32
                }

                Text {
                    id: eventText
                    anchors.top: parent.top
                    anchors { left: eventPic.right; right: dismissButton.left }
                    anchors { leftMargin: 2; rightMargin: 2 }

                    text: object.extendedText
                    color: "lightgrey"
                }

                TextButton {
                    id: dismissButton
                    anchors { right: parent.right; top: parent.top }
 
                    text: "x"
                    onClicked: { notifArea.eventDismissed (object.eventID) }
                }

            }

            Flow {
                id: actionsArea
                anchors { left: parent.left; right: parent.right }
                anchors.top: eventInfoRow.bottom
                anchors.topMargin: 2

                Repeater {
                    model: object.eventActionsModel
                    delegate: TextButton {
                        text: modelData
                        onClicked: { notifArea.eventActionTriggered (eventRect.eventID, index) }
                    }
                }
            }
        }
    }
}
