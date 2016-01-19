import QtQuick 2.0
import org.LC.common 1.0

Item {
    id: actionRoot

    property bool isHighlight
    property bool isStrongHighlight

    property bool isCurrent

    property string actionIconURL
    property bool actionIconScales: true

    property bool hoverScalesIcons: true

    property alias isHovered: actionMouseArea.containsMouse
    property alias acceptedButtons: actionMouseArea.acceptedButtons

    property string actionText
    property string textTooltip
    property string overlayText

    property string orientation: "horizontal"
    property bool isButtonVertical: orientation == "vertical"

    property bool forceHover: false
    property bool forcePress: false

    signal triggered()
    signal clicked(variant event)
    signal hovered()
    signal hoverLeft()
    signal held()

    property bool marginsManaged: false
    property alias margins: actionRect.margins

    Rectangle {
        id: actionRect

        radius: Math.min(width, height) / 10
        smooth: true

        property real margins: 0

        property bool hasText : actionRoot.actionText.length > 0

        anchors.fill: parent
        anchors.margins: actionRoot.marginsManaged ? 0 : margins
        border.width: isStrongHighlight ? 2 : 1

        property real decoOpacity: actionRect.hasText ? 0.5 : 0

        border.color: colorProxy.setAlpha(colorProxy.color_ToolButton_BorderColor, decoOpacity)

        gradient: Gradient {
            GradientStop {
                id: topGradientStop
                position: 0
                color: colorProxy.setAlpha(colorProxy.color_ToolButton_TopColor, actionRect.decoOpacity)
            }
            GradientStop {
                id: bottomGradientStop
                position: 1
                color: colorProxy.setAlpha(colorProxy.color_ToolButton_BottomColor, actionRect.decoOpacity)
            }
        }

        states: [
            State {
                name: "current"
                when: actionRoot.isCurrent && !actionMouseArea.containsMouse && !forceHover && !forcePress
                PropertyChanges { target: topGradientStop; color: colorProxy.color_ToolButton_SelectedTopColor }
                PropertyChanges { target: bottomGradientStop; color: colorProxy.color_ToolButton_SelectedBottomColor }
            },
            State {
                name: "hovered"
                when: (actionMouseArea.containsMouse && !actionMouseArea.pressed) || actionRoot.forceHover
                PropertyChanges { target: actionRect; border.color: colorProxy.color_ToolButton_HoveredBorderColor }
                PropertyChanges { target: topGradientStop; color: colorProxy.color_ToolButton_HoveredTopColor }
                PropertyChanges { target: bottomGradientStop; color: colorProxy.color_ToolButton_HoveredBottomColor }
            },
            State {
                name: "pressed"
                when: (actionMouseArea.containsMouse && actionMouseArea.pressed) || actionRoot.forcePress
                PropertyChanges { target: actionRect; border.color: colorProxy.color_ToolButton_PressedBorderColor }
            }
        ]

        transitions: [
            Transition {
                from: ""
                to: "hovered,current"
                reversible: true
                PropertyAnimation { properties: "border.color,color"; duration: 200 }
            }
        ]

        Image {
            id: actionImageElem

            anchors.top: parent.top
            anchors.left: parent.left
            width: Math.min(parent.width, parent.height)
            height: width

            source: actionIconURL.length > 0 && actionIconScales ?
                        actionIconURL + '/' + width :
                        actionIconURL
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

            visible: !isButtonVertical && actionText.length > 0

            text: actionText
            elide: Text.ElideRight
            font.pointSize: 7
            color: colorProxy.color_ToolButton_TextColor
        }

        MouseArea {
            id: actionMouseArea

            anchors.fill: parent
            hoverEnabled: true

            ToolTip {
                anchors.fill: parent
                text: textTooltip
            }

            onClicked: mouse.button == Qt.LeftButton ? actionRoot.triggered() : actionRoot.clicked(mouse)
            onPressAndHold: actionRoot.held()
            onEntered: actionRoot.hovered();
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

        width: parent.height / 12
        height: parent.width / 1.5
        radius: 1

        rotation: isButtonVertical ? 0 : 90

        anchors.horizontalCenter: isButtonVertical ? undefined : parent.horizontalCenter
        anchors.left: isButtonVertical ? parent.left : undefined
        anchors.verticalCenter: isButtonVertical ? parent.verticalCenter : parent.bottom
        anchors.verticalCenterOffset: isButtonVertical ? 0 : (-width / 2)

        gradient: Gradient {
            GradientStop {
                position: 0
                color: colorProxy.setAlpha(colorProxy.color_ToolButton_SelectedBorderColor, 0.2)
            }
            GradientStop {
                position: 0.5
                color: colorProxy.color_ToolButton_SelectedBorderColor
            }
            GradientStop {
                position: 1
                color: colorProxy.setAlpha(colorProxy.color_ToolButton_SelectedBorderColor, 0.2)
            }
        }
    }

    Rectangle {
        id: highlightCurrentRect

        opacity: isStrongHighlight ? 1 : 0
        Behavior on opacity { PropertyAnimation {} }

        width: parent.height / 12
        height: parent.width / 1.5
        radius: 1

        rotation: isButtonVertical ? 0 : 90

        anchors.horizontalCenter: isButtonVertical ? undefined : parent.horizontalCenter
        anchors.right: isButtonVertical ? parent.right : undefined
        anchors.verticalCenter: isButtonVertical ? parent.verticalCenter : parent.top
        anchors.verticalCenterOffset: isButtonVertical ? 0 : (width / 2)

        gradient: Gradient {
            GradientStop {
                position: 0
                color: colorProxy.setAlpha(colorProxy.color_ToolButton_SelectedBorderColor, 0.2)
            }
            GradientStop {
                position: 0.5
                color: colorProxy.color_ToolButton_SelectedBorderColor
            }
            GradientStop {
                position: 1
                color: colorProxy.setAlpha(colorProxy.color_ToolButton_SelectedBorderColor, 0.2)
            }
        }
    }
}
