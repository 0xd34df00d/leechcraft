import QtQuick 1.1
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real itemSize: parent.quarkBaseSize
    property real length: sensorsView.count * itemSize
    width: viewOrient == "vertical" ? itemSize : length
    height: viewOrient == "vertical" ? length : itemSize

    radius: 2

    color: "transparent"

    property variant tooltip

    Common { id: commonJS }

    ListView {
        id: sensorsView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds

        model: HS_sensorsModel

        orientation: viewOrient == "vertical" ? ListView.Vertical : ListView.Horizontal

        delegate: Item {
            id: delegateItem

            height: rootRect.itemSize
            width: rootRect.itemSize

            Image {
                id: sensorImage

                height: rootRect.itemSize
                width: rootRect.itemSize

                sourceSize.width: width
                sourceSize.height: height

                source: "data:," + rawSvg
            }

            Text {
                anchors.centerIn: parent
                text: lastTemp
                font.pixelSize: parent.height / 3
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true

                onEntered: {
                    var global = commonJS.getTooltipPos(delegateItem);
                    var params = {
                        "x": global.x,
                        "y": global.y,
                        "existing": "ignore",
                        "svg": rawSvg,
                        "colorProxy": colorProxy,
                        "sensorName": sensorName
                    };
                    tooltip = quarkProxy.openWindow(sourceURL, "Tooltip.qml", params);
                    tooltip.svg = (function() { return rawSvg; });
                }
                onExited: if (tooltip != null) { tooltip.closeRequested(); tooltip = null; }
            }

            ListView.onRemove: if (tooltip != null) { tooltip.closeRequested(); tooltip = null; }

            ActionButton {
                id: removeButton

                visible: quarkDisplayRoot.settingsMode
                opacity: 0

                width: parent.width / 2
                height: parent.height / 2
                anchors.top: parent.top
                anchors.right: parent.right

                actionIconURL: "image://ThemeIcons/list-remove"
                transparentStyle: true

                onTriggered: HS_plotManager.sensorHideRequested(sensorName, quarkContext)

                states: [
                    State {
                        name: "active"
                        when: quarkDisplayRoot.settingsMode
                        PropertyChanges { target: removeButton; opacity: 1 }
                    }
                ]

                transitions: [
                    Transition {
                        from: ""
                        to: "active"
                        reversible: true
                        PropertyAnimation { properties: "opacity"; duration: 200 }
                    }
                ]
            }
        }
    }
}
