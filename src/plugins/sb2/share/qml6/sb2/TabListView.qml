import QtQuick 2.3
import org.LC.common 1.0

Rectangle {
    id: rootRect
    width: Math.min(Math.max(300, longestTextLabel.paintedWidth + 10), 600)
    height: Math.min(600, tabsView.count * 36)
    smooth: true
    radius: 5
    focus: true

    Text {
        id: longestTextLabel
        visible: false
        text: longestText
    }

    opacity: 0
    SequentialAnimation on opacity {
        loops: 1
        PropertyAnimation { to: 1; duration: 100 }
    }

    signal closeRequested()
    signal tabSwitchRequested(int index)
    signal tabCloseRequested(int index)

    Keys.onEscapePressed: rootRect.closeRequested()

    gradient: Gradient {
        GradientStop {
            position: 0
            color: colorProxy.color_TextView_TopColor
        }
        GradientStop {
            position: 1
            color: colorProxy.color_TextView_BottomColor
        }
    }

    ListView {
        id: tabsView
        anchors.fill: parent

        model: tabsListModel

        delegate: Item {
            width: tabsView.width
            height: 36

            Rectangle {
                id: tabRect
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
                    id: tabIconImage
                    source: tabIcon

                    width: 32
                    height: 32
                    smooth: true

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 12

                    cache: false
                }

                Text {
                    id: tabNameLabel
                    text: tabName

                    color: colorProxy.color_TextBox_TextColor

                    anchors.left: tabIconImage.right
                    anchors.leftMargin: 4
                    anchors.right: closeTabButton.right
                    anchors.rightMargin: 4
                    anchors.verticalCenter: parent.verticalCenter

                    elide: Text.ElideMiddle
                }

                MouseArea {
                    id: rectMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onReleased: rootRect.tabSwitchRequested(index)
                }

                ActionButton {
                    id: closeTabButton
                    z: rectMouseArea.z + 1

                    height: parent.height * 2 / 3
                    width: height

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 4

                    actionIconURL: "image://ThemeIcons/tab-close"

                    onTriggered: rootRect.tabCloseRequested(index)
                }

                states: [
                    State {
                        name: "hovered"
                        when: rectMouseArea.containsMouse
                        PropertyChanges { target: tabRect; border.color: colorProxy.color_TextBox_HighlightBorderColor }
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
