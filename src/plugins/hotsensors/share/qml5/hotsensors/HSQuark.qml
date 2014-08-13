import QtQuick 2.3
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real itemSize: parent.quarkBaseSize
    property real length: sensorsView.count * itemSize + (addSensorButton.visible ? addSensorButton.height : 0)
    implicitWidth: viewOrient == "vertical" ? itemSize : length
    implicitHeight: viewOrient == "vertical" ? length : itemSize

    radius: 2

    color: "transparent"

    property variant tooltip

    Common { id: commonJS }

    Component.onCompleted: HS_plotManager.setContext(quarkContext)

    ActionButton {
        id: addSensorButton
        visible: quarkDisplayRoot.settingsMode
        anchors.bottom: viewOrient == "vertical" ? parent.bottom : undefined
        anchors.right: viewOrient == "vertical" ? undefined : parent.right
        anchors.horizontalCenter: viewOrient == "vertical" ? parent.horizontalCenter : undefined
        anchors.verticalCenter: viewOrient == "vertical" ? undefined : parent.verticalCenter
        width: parent.itemSize * 2 / 3
        height: parent.itemSize * 2 / 3

        actionIconURL: "image://ThemeIcons/list-add"

        onTriggered: commonJS.showTooltip(addSensorButton, function(x, y) { HS_plotManager.sensorUnhideListRequested(x, y, quarkProxy.getWinRect()) })
    }

    ListView {
        id: sensorsView
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: viewOrient == "vertical" ? undefined : addSensorButton.left
        anchors.bottom: viewOrient == "vertical" ? addSensorButton.top : undefined
        boundsBehavior: Flickable.StopAtBounds

        model: HS_plotManager.getModel()

        orientation: viewOrient == "vertical" ? ListView.Vertical : ListView.Horizontal

        delegate: Item {
            id: delegateItem

            height: rootRect.itemSize
            width: rootRect.itemSize

            Plot {
                id: sensorImage
                height: rootRect.itemSize
                width: rootRect.itemSize

                points: pointsList

                minYValue: 0
                maxYValue: Math.max(maxTemp, critTemp)
            }

            Text {
                anchors.centerIn: parent
                text: lastTemp
                font.pixelSize: parent.height / 3
                color: colorProxy.color_Panel_TextColor
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true

                Common { id: opener }

                onEntered: {
                    var params = {
                        "colorProxy": colorProxy,
                        "maxTemp": maxTemp,
                        "critTemp": critTemp,
                        "sensorName": sensorName,
                        "pointsList": Qt.binding(function() { return pointsList; })
                    };
                    opener.openWindow(delegateItem, params, Qt.resolvedUrl("Tooltip.qml"), tooltip, function(t) { tooltip = t; });
                }
                onExited: { tooltip.destroy(); tooltip = null; }
            }

            ActionButton {
                id: removeButton

                visible: quarkDisplayRoot.settingsMode
                opacity: 0

                width: parent.width / 2
                height: parent.height / 2
                anchors.top: parent.top
                anchors.right: parent.right

                actionIconURL: "image://ThemeIcons/list-remove"

                onTriggered: HS_plotManager.hideSensor(sensorName)

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
