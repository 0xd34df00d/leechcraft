import QtQuick 1.1
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real length: cpusView.count * 10
    implicitWidth: viewOrient == "vertical" ? parent.quarkBaseSize : length
    implicitHeight: viewOrient == "vertical" ? length : parent.quarkBaseSize

    ListView {
        id: cpusView

        width: rootRect.parent.quarkBaseSize
        height: parent.length
        interactive: false

        model: CpuLoad_model

        transform: Rotation {
            origin.x: 0
            origin.y: 0
            axis { x: 1; y: 1; z: 0 }

            angle: viewOrient == "horizontal" ? 0 : 180
        }

        delegate: Item {
            height: cpusView.height
            width: 10

            Rectangle {
                id: highPercRect
                height: cpusView.height * loadObj.highPercentage
                width: parent.width
                color: "red"

                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
            }

            Rectangle {
                id: mediumPercRect
                height: cpusView.height * loadObj.mediumPercentage
                width: parent.width
                color: "blue"

                anchors.bottom: highPercRect.top
                anchors.left: parent.left
                anchors.right: parent.right
            }

            Rectangle {
                id: lowPercRect
                height: cpusView.height * loadObj.lowPercentage
                width: parent.width
                color: "yellow"

                anchors.bottom: mediumPercRect.top
                anchors.left: parent.left
                anchors.right: parent.right
            }

            Rectangle {
                id: ioPercRect
                height: cpusView.height * loadObj.ioPercentage
                width: parent.width
                color: "green"

                anchors.bottom: lowPercRect.top
                anchors.left: parent.left
                anchors.right: parent.right
            }
        }
    }
}
