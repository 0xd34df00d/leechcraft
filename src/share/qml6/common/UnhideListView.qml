import QtQuick 2.3
import org.LC.common 1.0

Rectangle {
    id: rootRect
    width: 500
    height: Math.min(600, unhideView.count * 36)
    smooth: true
    focus: true

    opacity: 0
    SequentialAnimation on opacity {
        loops: 1
        PropertyAnimation { to: 1; duration: 100 }
    }

    signal closeRequested()
    signal itemUnhideRequested(string itemClass)

    Keys.onEscapePressed: rootRect.closeRequested()

    color: "transparent"

    ListView {
        id: unhideView
        anchors.fill: parent
        model: unhideListModel

        delegate: Item {
            width: unhideView.width
            height: 36

            Rectangle {
                id: itemRect
                anchors.fill: parent
                radius: 5
                smooth: true

                border.color: colorProxy.color_TextBox_BorderColor
                border.width: 1

                Keys.onEscapePressed: rootRect.closeRequested()

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
                    onReleased: rootRect.itemUnhideRequested(itemClass)
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
