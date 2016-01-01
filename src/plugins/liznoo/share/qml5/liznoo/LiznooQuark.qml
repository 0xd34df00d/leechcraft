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
                    height: parent.height
                    width: height / 1.62

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top

                    radius: 3
                    color: "transparent"
                    border.width: 1
                    border.color: colorProxy.color_TextBox_BorderColor

                    Rectangle {
                        anchors.fill: parent
                        anchors.leftMargin: 2
                        anchors.rightMargin: 2
                        anchors.bottomMargin: 2
                        anchors.topMargin: 2 + (parent.height - 4) * (1 - percentage / 100)

                        radius: parent.radius
                        color: "transparent"

                        Rectangle {
                            width: parent.height
                            height: parent.width

                            anchors.top: parent.top
                            anchors.left: parent.left

                            transform: Rotation { axis.x: 1; axis.y: 1; axis.z: 0; angle: 180 }

                            radius: parent.radius

                            gradient: Gradient {
                                GradientStop { position: 0.0; color: colorProxy.color_TextBox_BottomColor }
                                GradientStop { position: 1.0; color: colorProxy.color_TextBox_TopColor }
                            }
                        }

                        Text {
                            color: colorProxy.color_TextBox_TextColor
                            text: percentage + '%'
                            font.pointSize: 5

                            anchors.centerIn: parent
                        }
                    }
                }
            }
        }
    }
}
