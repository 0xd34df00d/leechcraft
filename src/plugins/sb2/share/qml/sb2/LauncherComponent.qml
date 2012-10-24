import QtQuick 1.1
import Effects 1.0
import "."

Rectangle {
    id: rootRect

    width: parent.width
    property real launcherItemHeight: parent.width
    property real currentGapSize: (launcherItemHeight + Math.sqrt(8 * launcherItemHeight)) / 4
    height: launcherColumn.height

    color: "transparent"

    Column {
        id: launcherColumn
        Repeater {
            model: SB2_launcherModel
            Item {
                id: tcItem

                height: rootRect.launcherItemHeight + pregap.height + postgap.height
                width: rootRect.width

                Item {
                    id: pregap
                    height: isCurrentTab ? rootRect.currentGapSize : 0
                    Behavior on height { PropertyAnimation { duration: 200 } }
                    anchors.top: parent.top
                }

                ActionButton {
                    id: tcButton
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: pregap.bottom

                    height: rootRect.launcherItemHeight

                    z: 5

                    actionIconURL: tabClassIcon
                    isHighlight: openedTabsCount
                    isStrongHighlight: openedTabsCount
                    isCurrent: isCurrentTab

                    onTriggered: SB2_launcherProxy.tabOpenRequested(tabClassID)

                    effect: Colorize {
                        strength: openedTabsCount ? 0 : 0.3
                        color: "gray"

                        Behavior on strength { PropertyAnimation {} }
                    }
                }

                Item {
                    id: postgap
                    height: isCurrentTab ? rootRect.currentGapSize : 0
                    Behavior on height { PropertyAnimation { duration: 200 } }
                    anchors.bottom: parent.bottom
                }
            }
        }
    }
}
