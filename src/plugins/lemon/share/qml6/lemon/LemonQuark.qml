import QtQuick
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real itemSize: parent.quarkBaseSize
    implicitWidth: viewOrient == "vertical" ? itemSize : (indicatorsView.count * itemSize)
    implicitHeight: viewOrient == "vertical" ? (indicatorsView.count * itemSize) : itemSize

    color: "transparent"

    property url tooltipUrl: Qt.resolvedUrl("Tooltip.qml")

    Common { id: opener }

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
                    const params = {
                        ifaceName: ifaceName,
                        upSpeed: Qt.binding(() => upSpeedPretty),
                        downSpeed: Qt.binding(() => downSpeedPretty),
                        colorProxy: colorProxy
                    };
                    opener.openTooltip(indicatorButton, params, tooltipUrl, ifaceName);
                }
                onHoverLeft: opener.closeTooltip(tooltipUrl, ifaceName)

                function adjustAlpha(c) {
                    return Qt.rgba(c.r, c.g, c.b, 0x22 / 0xff);
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    width: parent.width / 2
                    height: maxDownSpeed ? parent.height * downSpeed / maxDownSpeed : 0

                    color: indicatorButton.adjustAlpha(Lemon_proxy.downloadGraphColor)

                    visible: Lemon_showBars
                }

                Rectangle {
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    width: parent.width / 2
                    height: maxUpSpeed ? parent.height * upSpeed / maxUpSpeed : 0

                    color: indicatorButton.adjustAlpha(Lemon_proxy.uploadGraphColor)

                    visible: Lemon_showBars
                }

                Text {
                    id: downloadTextLabel

                    anchors.right: parent.right
                    anchors.top: parent.top
                    visible: Lemon_showTextLabels

                    color: Lemon_proxy.downloadGraphColor
                    font.pointSize: 6

                    text: quarkProxy.prettySizeShort(downSpeed)
                }

                Text {
                    id: uploadTextLabel

                    anchors.right: parent.right
                    anchors.top: downloadTextLabel.bottom
                    visible: Lemon_showTextLabels

                    color: Lemon_proxy.uploadGraphColor
                    font.pointSize: 6

                    text: quarkProxy.prettySizeShort(upSpeed)
                }
            }
        }
    }
}
