import QtQuick 2.3

Rectangle {
    property int targetHeight: trackListText.height + 10
    property alias text: trackListText.text

    id: trackListContainerRect
    z: 0
    opacity: 0

    radius: 5
    width: 400
    height: targetHeight
    clip: true

    color: colorProxy.setAlpha(colorProxy.color_TextBox_TopColor, 0.9)

    border.color: colorProxy.color_TextBox_HighlightBorderColor
    border.width: 1

    Text {
        id: trackListText
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5

        elide: Text.ElideRight
        color: colorProxy.color_TextBox_TextColor
    }

    states: [
        State {
            name: "visible"
            PropertyChanges { target: trackListContainerRect; z: 5; opacity: 1 }
        }
    ]

    Behavior on x { PropertyAnimation { duration: 200 } }
    Behavior on y { PropertyAnimation { duration: 200 } }
    Behavior on height { PropertyAnimation { duration: 200 } }

    transitions: Transition {
        ParallelAnimation {
            PropertyAnimation { property: "opacity"; duration: 300; easing.type: Easing.OutSine }
        }
    }
}
