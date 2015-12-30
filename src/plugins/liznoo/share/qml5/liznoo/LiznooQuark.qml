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
    }
}
