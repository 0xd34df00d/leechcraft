import QtQuick 1.1
import "lcqml:org/LC/common/Common.js" as Common

Item {
    id: actionRoot

    property bool isHighlight
    property bool isStrongHighlight

    property bool isCurrent

    property string actionIconURL
    property bool actionIconScales: true

    property bool hoverScalesIcons: true
    property bool transparentStyle: false

    property alias isHovered: actionMouseArea.containsMouse

    property string textTooltip

    property string overlayText

    signal triggered()
    signal hovered()
    signal hoverLeft()
    signal held()

    Rectangle {
        id: actionRect

        radius: width / 10
        smooth: true

        anchors.fill: parent
        anchors.margins: hoverScalesIcons ? 2 : 0
        border.width: transparentStyle ? 0 : (isStrongHighlight ? 2 : 1)
        border.color: actionRoot.isHighlight ? colorProxy.color_ToolButton_SelectedBorderColor : colorProxy.color_ToolButton_BorderColor

        gradient: Gradient {
            GradientStop {
                position: 0
                color: transparentStyle ? "#00000000" : (actionRoot.isHighlight ? colorProxy.color_ToolButton_SelectedTopColor : colorProxy.color_ToolButton_TopColor)
            }
            GradientStop {
                position: 1
                color: transparentStyle ? "#00000000" : (actionRoot.isHighlight ? colorProxy.color_ToolButton_SelectedBottomColor : colorProxy.color_ToolButton_BottomColor)
            }
        }

        states: [
            State {
                name: "current"
                when: actionRoot.isCurrent && !actionMouseArea.containsMouse
                PropertyChanges { target: actionRect; anchors.margins: 0 }
            },
            State {
                name: "hovered"
                when: actionMouseArea.containsMouse && !actionMouseArea.pressed
                PropertyChanges { target: actionRect; border.color: colorProxy.color_ToolButton_HoveredBorderColor; anchors.margins: 0 }
            },
            State {
                name: "pressed"
                when: actionMouseArea.containsMouse && actionMouseArea.pressed
                PropertyChanges { target: actionRect; border.color: colorProxy.color_ToolButton_PressedBorderColor }
            }
        ]

        transitions: [
            Transition {
                from: ""
                to: "hovered,current"
                reversible: true
                PropertyAnimation { properties: "border.color,anchors.margins"; duration: 200 }
            }
        ]

        Image {
            id: actionImageElem

            anchors.fill: parent

            source: actionIconScales ? (actionIconURL + '/' + width) : actionIconURL
            smooth: true
            cache: false
        }

        MouseArea {
            id: actionMouseArea

            anchors.fill: parent
            hoverEnabled: true

            onClicked: actionRoot.triggered()
            onPressAndHold: actionRoot.held()
            onEntered: {
                actionRoot.hovered();
                if (textTooltip.length > 0)
                    Common.showTextTooltip(actionMouseArea, textTooltip);
            }
            onExited: actionRoot.hoverLeft()
        }
    }

    Text {
        id: overlayTextLabel

        z: parent.z + 2
        anchors.right: parent.right
        anchors.rightMargin: parent.width / 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height / 10

        text: overlayText
        visible: overlayText.length > 0

        color: "white"
        font.pixelSize: parent.height / 3
        font.italic: true
        smooth: true

        verticalAlignment: Text.AlignBottom
        horizontalAlignment: Text.AlignRight
    }

    Rectangle {
        z: overlayTextLabel.z - 1
        anchors.fill: overlayTextLabel
        anchors.topMargin: -2
        anchors.leftMargin: -parent.width / 10
        anchors.bottomMargin: -1

        visible: overlayTextLabel.visible

        color: "#E01919"
        smooth: true
        radius: 4
        border.color: "white"
        border.width: 1
    }
}
