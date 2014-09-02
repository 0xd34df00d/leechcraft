import QtQuick 2.3
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

                Common { id: opener }

                onTriggered: Lemon_proxy.showGraph(ifaceName)
                onHovered: {
                    var params = {
                        "ifaceName": ifaceName,
                        "upSpeed": Qt.binding(function() { return upSpeedPretty; }),
                        "downSpeed": Qt.binding(function() { return downSpeedPretty; }),
                        "colorProxy": colorProxy
                    };
                    opener.openWindow(indicatorButton, params, Qt.resolvedUrl("Tooltip.qml"), tooltip, function(t) { tooltip = t; });
                }
                onHoverLeft: if (tooltip != null) tooltip.destroy()

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
