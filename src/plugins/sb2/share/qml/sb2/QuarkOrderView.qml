import QtQuick 1.1
import "../common/"

Rectangle {
    id: rootRect
    width: 500
    height: Math.min(600, closeItemButton.height + quarkListView.count * 36)
    smooth: true
    focus: true

    opacity: 0
    SequentialAnimation on opacity {
        loops: 1
        PropertyAnimation { to: 1; duration: 100 }
    }

    signal closeRequested()
    signal moveRequested(string from, string to, int shift)

    Keys.onEscapePressed: rootRect.closeRequested()

    color: "transparent"

    ActionButton {
        id: closeItemButton

        width: 16
        height: 16

        anchors.top: parent.top
        anchors.right: parent.right

        actionIconURL: "image://ThemeIcons/tab-close"
        transparentStyle: true

        onTriggered: rootRect.closeRequested()
    }

    Column {
        id: quarkListColumn
        anchors.top: closeItemButton.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        Repeater {
            id: quarkListView
            model: quarkListModel

            Rectangle {
                id: itemRect
                width: quarkListColumn.width
                height: 36
                z: 1
                radius: 5
                smooth: true

                border.color: colorProxy.color_TextBox_BorderColor
                border.width: 1

                Keys.onEscapePressed: rootRect.closeRequested()

                property int yBeforeDrag
                property string internalId: itemClass

                function savePos() { yBeforeDrag = y }
                function restorePos() { y = yBeforeDrag }

                function moveTo(other) {
                    restorePos()
                    rootRect.moveRequested(internalId, other.internalId, 0)
                }

                gradient: Gradient {
                    GradientStop {
                        position: 0
                        id: upperStop
                        color: colorProxy.color_TextBox_TopColor
                    }
                    GradientStop {
                        position: 1
                        id: lowerStop
                        color: colorProxy.color_TextBox_BottomColor
                    }
                }

                Image {
                    id: itemIconImage
                    source: itemIcon

                    width: 32
                    height: 32
                    smooth: true

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                }

                Text {
                    id: itemNameLabel
                    text: itemName + (itemDescr.length > 0 ? (" (" + itemDescr + ")") : "")

                    color: colorProxy.color_TextBox_TextColor

                    anchors.left: itemIconImage.right
                    anchors.leftMargin: 4
                    anchors.right: parent.right
                    anchors.rightMargin: 4
                    anchors.verticalCenter: parent.verticalCenter

                    elide: Text.ElideMiddle
                }

                MouseArea {
                    id: rectMouseArea
                    anchors.fill: parent
                    hoverEnabled: true

                    drag.target: itemRect
                    drag.axis: Drag.YAxis

                    onPressed: { itemRect.savePos(); itemRect.z = 2 }
                    onReleased: {
                        itemRect.z = 0

                        var quarkPt = mapToItem(quarkListColumn, mouseX, mouseY)
                        var other = quarkListColumn.childAt(quarkPt.x, quarkPt.y)

                        if (other !== null && other !== itemRect)
                            itemRect.moveTo(other)
                        else
                            itemRect.restorePos()

                        itemRect.z = 1
                    }
                }

                states: [
                    State {
                        name: "hovered"
                        when: rectMouseArea.containsMouse
                        PropertyChanges { target: itemRect; border.color: colorProxy.color_TextBox_HighlightBorderColor}
                        PropertyChanges { target: upperStop; color: colorProxy.color_TextBox_HighlightTopColor }
                        PropertyChanges { target: lowerStop; color: colorProxy.color_TextBox_HighlightBottomColor }
                    }
                ]

                transitions: [
                    Transition {
                        from: ""
                        to: "hovered"
                        reversible: true
                        PropertyAnimation { properties: "border.color"; duration: 200 }
                        PropertyAnimation { properties: "color"; duration: 200 }
                    }
                ]
            }
        }
    }
}

