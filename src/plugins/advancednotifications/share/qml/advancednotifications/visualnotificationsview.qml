import QtQuick 1.0
import "."

Rectangle {
    id: notifArea
    width: 450; height: 200
    smooth: true
    radius: 16
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#CF414141" }
        GradientStop { position: 1.0; color: "#CF1A1A1A" }
    }

    Component {
        id: actionsDelegate

        Rectangle {
            height: actionsListView.height
            width: actionText.width + 5
            smooth: true
            radius: 3
            color: "transparent"

            TextButton {
                id: actionText

                text: model.modelData.actionText
                onClicked: { model.modelData.actionSelected() }
            }
        }
    }

    Component {
        id: eventsDelegate

        Rectangle {
            id: eventRect

            width: listView.width
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
                    onClicked: { model.modelData.dismissEvent() }
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
                    delegate: actionsDelegate
                }
            }
        }
    }

    ListView {
        id: listView

        anchors.centerIn: parent
        anchors.topMargin: 5
        width: notifArea.width - 10
        height: notifArea.height

        model: eventsModel
        delegate: eventsDelegate
    }
}
