import QtQuick
import QtQuick.Controls
import org.LC.common 1.0 as LCCommon

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

        delegate: ProgressBar {
            from: 0
            to: jobTotal
            value: jobDone

            width: parent.width
            height: 10
        }
    }

    LCCommon.Common { id: commonJS }

    readonly property url tooltipUrl: Qt.resolvedUrl("Tooltip.qml")

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onEntered: {
            const params = {
                infoModel: TPI_infoModel,
                colorProxy: colorProxy
            };
            commonJS.openWindow(rootRect, params, tooltipUrl);
        }
        onExited: commonJS.closeTooltip(tooltipUrl)
    }
}
