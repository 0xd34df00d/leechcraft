import QtQuick 2.3
import QtQuick.Window 2.1

Window {
    property string downSpeed
    property string upSpeed

    width: rootRect.width
    height: rootRect.height

    flags: Qt.ToolTip

    Rectangle {
        id: rootRect
        width: childrenRect.width + ifaceNameLabel.anchors.leftMargin * 2
        height: childrenRect.height + ifaceNameLabel.anchors.topMargin * 2

        smooth: true
        radius: 5

        signal closeRequested()

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

        Text {
            id: ifaceNameLabel
            text: ifaceName

            anchors.top: parent.top
            anchors.topMargin: 5
            anchors.left: parent.left
            anchors.leftMargin: 5

            color: colorProxy.color_TextView_TitleTextColor
            font.bold: true
        }

        Text {
            id: downSpeedLabel
            text: downSpeed

            anchors.top: ifaceNameLabel.bottom
            anchors.left: parent.left
            anchors.leftMargin: 5

            color: colorProxy.color_TextView_TextColor
        }

        Text {
            id: upSpeedLabel
            text: upSpeed

            anchors.top: downSpeedLabel.bottom
            anchors.left: parent.left
            anchors.leftMargin: 5

            color: colorProxy.color_TextView_TextColor
        }
    }
}
