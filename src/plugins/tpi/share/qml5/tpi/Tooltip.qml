import QtQuick 2.3
import QtQuick.Window 2.1
import org.LC.common 1.0

Window {
    property variant colorProxy
    property variant infoModel

    width: rootRect.width
    height: rootRect.height

    flags: Qt.ToolTip

    Rectangle {
        id: rootRect
        width: 600
        height: tasksView.count * rowHeight
        color: "transparent"

        property real rowHeight: 37

        ListView {
            id: tasksView

            anchors.fill: parent
            interactive: false

            model: infoModel

            delegate: Rectangle {
                width: parent.width
                height: rootRect.rowHeight

                gradient: Gradient {
                    GradientStop {
                        position: 0
                        id: upperStop
                        color: colorProxy.color_TextBox_TopColor
                    }
                    GradientStop {
                        position: 1
                        id: lowerStop
                        color: colorProxy.color_TextBox_BottomColor
                    }
                }

                Text {
                    id: jobNameLabel
                    text: jobName + ": " + Math.floor(100 * jobDone / jobTotal) + "%"

                    color: colorProxy.color_TextBox_TextColor

                    anchors.top: parent.top
                    anchors.topMargin: 5
                    anchors.left: parent.left
                    anchors.leftMargin: 3
                    anchors.right: parent.right
                    anchors.rightMargin: 3
                }

                ProgressBar {
                    minimum: 0
                    maximum: jobTotal
                    value: jobDone

                    color: colorProxy.color_TextBox_Aux1TextColor

                    anchors.top: jobNameLabel.bottom
                    anchors.topMargin: 3
                    anchors.left: parent.left
                    anchors.leftMargin: 3
                    anchors.right: parent.right
                    anchors.rightMargin: 3
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 5
                }
            }
        }
    }
}
