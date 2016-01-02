import QtQuick 2.3
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real length: battView.count * parent.quarkBaseSize
    implicitWidth: viewOrient == "vertical" ? parent.quarkBaseSize : length
    implicitHeight: viewOrient == "vertical" ? length : parent.quarkBaseSize
    width: parent.quarkBaseSize
    height: parent.quarkBaseSize

    color: "transparent"

    Common { id: commonJS }

    ListView {
        id: battView

        width: parent.length
        height: rootRect.parent.quarkBaseSize
        interactive: false

        orientation: ListView.Horizontal

        model: Liznoo_proxy.batteryModel

        transform: Rotation {
            origin.x: 0
            origin.y: 0
            axis { x: 1; y: 1; z: 0 }

            angle: viewOrient == "horizontal" ? 0 : 180
        }

        delegate: Item {
            height: battView.height
            width: battView.height

            ActionButton {
                anchors.fill: parent

                marginsManaged: true
                anchors.margins: margins

                Rectangle {
                    width: parent.width - anchors.rightMargin
                    height: width / 1.62

                    anchors.centerIn: parent
                    anchors.rightMargin: 2

                    radius: 3
                    border.width: 1
                    border.color: colorProxy.color_ToolButton_SelectedBorderColor

                    gradient: Gradient {
                        GradientStop { position: 0.0; color: colorProxy.color_ToolButton_TopColor }
                        GradientStop { position: 1.0; color: colorProxy.color_ToolButton_BottomColor }
                    }

                    Rectangle {
                        anchors.fill: parent
                        property real baseMargin: 1
                        anchors.leftMargin: baseMargin
                        anchors.bottomMargin: baseMargin
                        anchors.topMargin: baseMargin
                        anchors.rightMargin: baseMargin + (parent.width - 2 * baseMargin) * (1 - percentage / 100)

                        radius: parent.radius

                        gradient: Gradient {
                            GradientStop { position: 0.0; color: colorProxy.color_ToolButton_HoveredBottomColor }
                            GradientStop { position: 1.0; color: colorProxy.color_ToolButton_HoveredTopColor }
                        }
                    }

                    Rectangle {
                        anchors.left: parent.right
                        width: parent.anchors.rightMargin
                        height: parent.height / 3
                        anchors.verticalCenter: parent.verticalCenter

                        radius: parent.radius

                        color: colorProxy.color_ToolButton_SelectedBorderColor
                    }

                    Text {
                        color: colorProxy.color_ToolButton_TextColor
                        text: percentage + '%'
                        font.pointSize: 7

                        anchors.centerIn: parent
                    }
                }
            }
        }
    }
}
