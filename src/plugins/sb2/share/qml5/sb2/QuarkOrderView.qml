import QtQuick 2.3
import org.LC.common 1.0

Rectangle {
    id: rootRect
    width: 500
    height: Math.min(600, quarkListView.count * 36)
    smooth: true
    focus: true

    opacity: 0
    SequentialAnimation on opacity {
        loops: 1
        PropertyAnimation { to: 1; duration: 100 }
    }

    signal closeRequested()
    signal moveRequested(string from, string to, int shift)
    signal quarkRemoveRequested(string classID)
    signal quarkClassHovered(string classID)

    Keys.onEscapePressed: rootRect.closeRequested()

    color: "transparent"

    Column {
        id: quarkListColumn
        anchors.fill: parent
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

                function moveTo(other, releasePt) {
                    restorePos();

                    var otherPt = other.mapFromItem(quarkListColumn, releasePt.x, releasePt.y);

                    var shift = otherPt.y <= other.height / 2 ? 0 : 1;
                    rootRect.moveRequested(internalId, other.internalId, shift);
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

                    cache: false
                }

                Text {
                    id: itemNameLabel
                    text: itemName + (itemDescr.length > 0 ? (" (" + itemDescr + ")") : "")

                    color: colorProxy.color_TextBox_TextColor

                    anchors.left: itemIconImage.right
                    anchors.leftMargin: 4
                    anchors.right: removeQuarkButton.left
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

                    onPressed: {
                        itemRect.savePos();
                        itemRect.z = 2;
                        itemRect.opacity = 0.7;
                    }
                    onReleased: {
                        itemRect.z = 0;

                        var quarkPt = mapToItem(quarkListColumn, mouseX, mouseY);
                        var other = quarkListColumn.childAt(quarkPt.x, quarkPt.y);

                        if (other !== null && other !== itemRect)
                            itemRect.moveTo(other, quarkPt);
                        else
                            itemRect.restorePos();

                        itemRect.z = 1;

                        itemRect.opacity = 1;
                    }

                    onEntered: rootRect.quarkClassHovered(itemClass)
                    onExited: rootRect.quarkClassHovered("")
                }

                ActionButton {
                    id: removeQuarkButton
                    z: rectMouseArea.z + 1

                    height: parent.height * 2 / 3
                    width: height

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 4

                    actionIconURL: "image://ThemeIcons/edit-delete"

                    onTriggered: rootRect.quarkRemoveRequested(itemClass)
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

