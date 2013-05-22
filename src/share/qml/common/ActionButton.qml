import QtQuick 1.1

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

    property string actionText
    property string textTooltip
    property string overlayText

    property string orientation: "horizontal"

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
        border.color: colorProxy.color_ToolButton_BorderColor

        gradient: Gradient {
            GradientStop {
                position: 0
                color: transparentStyle ? "#00000000" : colorProxy.color_ToolButton_TopColor
            }
            GradientStop {
                position: 1
                color: transparentStyle ? "#00000000" : colorProxy.color_ToolButton_BottomColor
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

            anchors.top: parent.top
            anchors.left: parent.left
            width: Math.min(parent.width, parent.height)
            height: width

            source: actionIconScales ? (actionIconURL + '/' + width) : actionIconURL
            smooth: true
            cache: false

            fillMode: Image.PreserveAspectFit
        }

        Text {
            id: actionTextElem

            anchors.verticalCenter: actionImageElem.verticalCenter
            anchors.left: actionImageElem.right
            anchors.leftMargin: 2
            anchors.right: parent.right

            visible: orientation != "vertical" && actionText.length > 0

            text: actionText
            elide: Text.ElideRight

            font.pointSize: 7
        }

        Common { id: buttCommon }

        MouseArea {
            id: actionMouseArea

            anchors.fill: parent
            hoverEnabled: true

            onClicked: actionRoot.triggered()
            onPressAndHold: actionRoot.held()
            onEntered: {
                actionRoot.hovered();
                if (textTooltip.length > 0)
                    buttCommon.showTextTooltip(actionMouseArea, textTooltip);
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

    Rectangle {
        id: highlightOpenedRect

        opacity: isHighlight ? 1 : 0
        Behavior on opacity { PropertyAnimation {} }

        width: orientation == "vertical" ? parent.height / 12 : parent.width / 2
        height: orientation == "vertical" ? parent.width / 2 : parent.height / 12
        radius: 1

        anchors.bottom: orientation == "vertical" ? undefined : parent.bottom
        anchors.horizontalCenter: orientation == "vertical" ? undefined : parent.horizontalCenter
        anchors.left: orientation == "vertical" ? parent.left : undefined
        anchors.verticalCenter: orientation == "vertical" ? parent.verticalCenter : undefined

        color: colorProxy.color_ToolButton_SelectedBorderColor
    }

    Rectangle {
        id: highlightCurrentRect

        opacity: isStrongHighlight ? 1 : 0
        Behavior on opacity { PropertyAnimation {} }

        width: orientation == "vertical" ? parent.height / 12 : parent.width / 2
        height: orientation == "vertical" ? parent.width / 2 : parent.height / 12
        radius: 1

        anchors.top: orientation == "vertical" ? undefined : parent.top
        anchors.horizontalCenter: orientation == "vertical" ? undefined : parent.horizontalCenter
        anchors.right: orientation == "vertical" ? parent.right : undefined
        anchors.verticalCenter: orientation == "vertical" ? parent.verticalCenter : undefined

        color: colorProxy.color_ToolButton_SelectedBorderColor
    }
}
