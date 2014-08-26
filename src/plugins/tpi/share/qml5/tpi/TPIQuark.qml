import QtQuick 2.3
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real length: tasksView.count * 10
    implicitWidth: viewOrient == "vertical" ? parent.quarkBaseSize : length
    implicitHeight: viewOrient == "vertical" ? length : parent.quarkBaseSize

    property variant tooltip: null

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

    Common { id: commonJS }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onEntered: {
            var params = {
                "infoModel": TPI_infoModel,
                "colorProxy": colorProxy
            };
            commonJS.openWindow(rootRect, params, Qt.resolvedUrl("Tooltip.qml"), tooltip, function(t) { tooltip = t; });
        }
        onExited: if (tooltip != null) tooltip.destroy()
    }
}
