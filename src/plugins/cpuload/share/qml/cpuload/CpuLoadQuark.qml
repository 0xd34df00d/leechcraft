import QtQuick 1.1
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real length: cpusView.count * 10
    implicitWidth: viewOrient == "vertical" ? parent.quarkBaseSize : length
    implicitHeight: viewOrient == "vertical" ? length : parent.quarkBaseSize

    ListView {
        id: cpusView

        width: rootRect.parent.childAt()quarkBaseSize
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
            Rectangle {
                height: cpusView.height * loadObj.highPercentage
                width: 10
                color: "red"
            }

            Rectangle {
                height: cpusView.height * loadObj.mediumPercentage
                width: 10
                color: "blue"
            }

            Rectangle {
                height: cpusView.height * loadObj.lowPercentage
                width: 10
                color: "yellow"
            }

            Rectangle {
                height: cpusView.height * loadObj.ioPercentage
                width: 10
                color: "green"
            }
        }
    }
}
