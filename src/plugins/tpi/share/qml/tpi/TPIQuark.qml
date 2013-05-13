import QtQuick 1.1
import org.LC.common 1.0
import "."
import "lcqml:org/LC/common/Common.js" as Common

Rectangle {
    id: rootRect

    property real length: tasksView.count * 10
    width: viewOrient == "vertical" ? parent.quarkBaseSize : length
    height: viewOrient == "vertical" ? length : parent.quarkBaseSize

    ListView {
        id: tasksView

        width: parent.parent.quarkBaseSize
        height: parent.length
        interactive: false

        model: TPI_infoModel

        transform: Rotation {
            origin.x: 0
            origin.y: 0
            axis { x: 1; y: 1; z: 0 }

            angle: viewOrient == "vertical" ? 0 : 180
        }

        delegate: Rectangle {
            width: parent.width
            height: 10

            border.color: "#777777"
            border.width: 1

            ProgressBar {
                minimum: 0
                maximum: jobTotal
                value: jobDone

                color: "#999999"

                anchors.fill: parent
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onEntered: Common.showTooltip(rootRect, function(x, y) { TPI_proxy.hovered(x, y, quarkProxy.getWinRect()) })
        onExited: TPI_proxy.hoverLeft()
    }
}
