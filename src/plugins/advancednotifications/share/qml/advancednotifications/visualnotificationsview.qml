import QtQuick 1.0
import "."

Rectangle {
    id: notifArea
    width: 450
    height: Math.min(200, listView.count * 61)
    smooth: true
    radius: 5
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#CF414141" }
        GradientStop { position: 1.0; color: "#CF1A1A1A" }
    }

    Component {
        id: eventsDelegate
        Rectangle {
            id: eventRect

            width: listView.width
            height: eventPic.height + 4 + actionsListView.height + 5
            smooth: true
            radius: 5
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#DF3A3A3A" }
                GradientStop { position: 1.0; color: "#DF101010" }
            }

            Image {
                id: eventPic
                source: image

                height: 32
                width: 32
                smooth: true

                anchors.top: parent.top
                anchors.topMargin: 2
                anchors.left: parent.left
                anchors.leftMargin: 12
            }

            Text {
                id: eventText

                width: parent.width - eventPic.width - eventPic.anchors.leftMargin - anchors.leftMargin - dismissButton.width - 10

                text: extendedText
                color: "lightgrey"

                anchors.top: parent.top
                anchors.topMargin: 2
                anchors.left: eventPic.right
                anchors.leftMargin: 4
            }

            TextButton {
                id: dismissButton

                text: "×"
                onClicked: { model.modelData.dismissEvent() }

                anchors.top: parent.top
                anchors.topMargin: 2
                anchors.right: parent.right
                anchors.rightMargin: 5
            }

            ListView {
                id: actionsListView

                height: 20
                width: parent.width
                anchors.left: parent.left
                anchors.leftMargin: eventPic.anchors.leftMargin
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 5

                spacing: 5

                orientation: ListView.Horizontal

                model: eventActionsModel
                delegate: actionsDelegate
            }
        }
    }

    Component {
        id: actionsDelegate

        Rectangle {
            height: actionsListView.height
            width: actionText.width
            smooth: true
            radius: 3
            color: "transparent"

            TextButton {
                id: actionText

                text: model.modelData.actionText
                onClicked: { model.modelData.actionSelected() }

                anchors.fill: parent
            }
        }
    }

    ListView {
        id: listView

        anchors.centerIn: parent
        width: notifArea.width
        height: notifArea.height

        model: eventsModel
        delegate: eventsDelegate
    }
}
