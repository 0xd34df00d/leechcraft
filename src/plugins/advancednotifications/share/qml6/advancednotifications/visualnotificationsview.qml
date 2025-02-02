import QtQuick
import QtQuick.Controls
import org.LC.common 1.0

Rectangle {
    id: notifArea
    width: 550
    height: Math.min(600, listView.contentHeight)
    smooth: true
    radius: 5
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

    Component {
        id: eventsDelegate

        Rectangle {
            id: eventRect

            width: listView.width
            height: childrenRect.height + 5

            smooth: true
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
            border.color: colorProxy.color_TextBox_BorderColor
            border.width: 1

            ActionButton {
                id: dismissButton

                actionIconURL: "image://ThemeIcons/dialog-close"
                onTriggered: model.modelData.dismissEvent()

                anchors.top: parent.top
                anchors.topMargin: 2
                anchors.right: parent.right
                anchors.rightMargin: 5
                width: 24
                height: 24
            }

            Item {
                id: eventPicAndText

                height: Math.max(eventPic.height, eventText.height)
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.right: dismissButton.left
                anchors.top: parent.top
                anchors.topMargin: 2

                Image {
                    id: eventPic
                    source: image

                    height: 32
                    width: 32
                    smooth: true

                    anchors.top: parent.top
                    anchors.left: parent.left
                }

                Text {
                    id: eventText

                    width: parent.width - eventPic.width - eventPic.anchors.leftMargin - anchors.leftMargin - dismissButton.width - 10

                    text: extendedText
                    color: colorProxy.color_TextBox_TextColor

                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    maximumLineCount: 4

                    anchors.top: parent.top
                    anchors.left: eventPic.right
                    anchors.leftMargin: 4
                }
            }

            ListView {
                id: actionsListView

                height: 20
                anchors.left: parent.left
                anchors.leftMargin: eventPicAndText.anchors.leftMargin
                anchors.right: parent.right
                anchors.top: eventPicAndText.bottom
                anchors.topMargin: count ? 10 : 0

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
            id: actionRect

            height: 20
            width: actionText.width
            smooth: true
            radius: 3
            color: "transparent"

            Button {
                id: actionText

                text: model.modelData.actionText
                onClicked: model.modelData.actionSelected()

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
