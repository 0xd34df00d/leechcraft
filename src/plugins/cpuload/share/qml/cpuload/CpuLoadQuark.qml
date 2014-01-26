import QtQuick 1.1
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real length: cpusView.count * 10
    implicitWidth: viewOrient == "vertical" ? parent.quarkBaseSize : length
    implicitHeight: viewOrient == "vertical" ? length : parent.quarkBaseSize

    color: "transparent"

    ListView {
        id: cpusView

        width: parent.length
        height: rootRect.parent.quarkBaseSize
        interactive: false

        orientation: ListView.Horizontal

        model: CpuLoad_model

        transform: Rotation {
            origin.x: 0
            origin.y: 0
            axis { x: 1; y: 1; z: 0 }

            angle: viewOrient == "horizontal" ? 0 : 180
        }

        delegate: Rectangle {
            id: delegateRoot

            height: cpusView.height
            width: 10

            color: "transparent"

            Rectangle {
                id: highPercRect
                height: cpusView.height * loadObj.highPercentage
                width: delegateRoot.width
                color: "red"

                anchors.bottom: delegateRoot.bottom
                anchors.left: delegateRoot.left
            }

            Rectangle {
                id: mediumPercRect
                height: cpusView.height * loadObj.mediumPercentage
                width: delegateRoot.width
                color: "blue"

                anchors.bottom: highPercRect.top
                anchors.left: delegateRoot.left
            }

            Rectangle {
                id: lowPercRect
                height: cpusView.height * loadObj.lowPercentage
                width: delegateRoot.width
                color: "yellow"

                anchors.bottom: mediumPercRect.top
                anchors.left: delegateRoot.left
            }

            Rectangle {
                id: ioPercRect
                height: cpusView.height * loadObj.ioPercentage
                width: delegateRoot.width
                color: "green"

                anchors.bottom: lowPercRect.top
                anchors.left: delegateRoot.left
            }
        }
    }
}
