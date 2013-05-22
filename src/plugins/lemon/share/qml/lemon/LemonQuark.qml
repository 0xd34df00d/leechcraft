import QtQuick 1.1
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real itemSize: parent.quarkBaseSize
    implicitWidth: viewOrient == "vertical" ? itemSize : (indicatorsView.count * itemSize)
    implicitHeight: viewOrient == "vertical" ? (indicatorsView.count * itemSize) : itemSize

    color: "transparent"

    property variant tooltip: null

    ListView {
        id: indicatorsView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds

        model: Lemon_infoModel

        orientation: viewOrient == "vertical" ? ListView.Vertical : ListView.Horizontal

        delegate: Rectangle {
            width: rootRect.itemSize
            height: rootRect.itemSize

            color: "transparent"

            ActionButton {
                id: indicatorButton

                anchors.fill: parent
                actionIconURL: "image://ThemeIcons/" + iconName + '/' + width

                onTriggered: Lemon_proxy.showGraph(ifaceName)
                onHovered: {
                    var global = commonJS.getTooltipPos(indicatorButton);
                    var params = {
                        "x": global.x,
                        "y": global.y,
                        "ifaceName": ifaceName,
                        "upSpeed": upSpeedPretty,
                        "downSpeed": downSpeedPretty,
                        "colorProxy": colorProxy
                    };
                    if (tooltip != null)
                        tooltip.closeRequested();

                    tooltip = quarkProxy.openWindow(sourceURL, "Tooltip.qml", params);

                    tooltip.upSpeed = (function() { return upSpeedPretty; })
                    tooltip.downSpeed = (function() { return downSpeedPretty; })
                }
                onHoverLeft: if (tooltip != null) tooltip.closeRequested()

                Rectangle {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    width: parent.width / 2
                    height: maxDownSpeed ? parent.height * downSpeed / maxDownSpeed : 0

                    color: "#44009900"
                }

                Rectangle {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    width: parent.width / 2
                    height: maxUpSpeed ? parent.height * upSpeed / maxUpSpeed : 0

                    color: "#44990000"
                }
            }
        }
    }
}
