import QtQuick 1.1
import Effects 1.0
import SB2 1.0
import "."

Rectangle {
    id: rootRect

    width: parent.width
    property real launcherItemHeight: parent.width
    property real currentGapSize: (launcherItemHeight + Math.sqrt(8 * launcherItemHeight)) / 4
    height: launcherColumn.height + 2 + (addTCButton.visible ? addTCButton.height : 0)

    border.width: 1
    border.color: "#333333"
    radius: 2

    smooth: true

    color: "transparent"

    ActionButton {
        id: addTCButton
        visible: quarkDisplayRoot.settingsMode
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width * 2 / 3
        height: parent.width * 2 / 3

        actionIconURL: "image://ThemeIcons/list-add"

        LauncherDropArea {
            id: dropArea
            anchors.fill: parent
            onTabDropped: SB2_launcherProxy.tabClassUnhideRequested(tabClass)
        }
    }

    Column {
        id: launcherColumn
        anchors.top: parent.top
        anchors.topMargin: 1
        Repeater {
            id: launcherItemRepeater
            model: SB2_launcherModel
            Item {
                id: tcItem

                height: rootRect.launcherItemHeight + pregap.height + postgap.height
                width: rootRect.width

                Item {
                    id: pregap
                    height: (index && isCurrentTab) ? rootRect.currentGapSize : 0
                    Behavior on height { PropertyAnimation { duration: 100 } }
                    anchors.top: parent.top
                }

                ActionButton {
                    id: tcButton
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: pregap.bottom

                    height: rootRect.launcherItemHeight

                    actionIconURL: tabClassIcon
                    isHighlight: openedTabsCount
                    isStrongHighlight: openedTabsCount
                    isCurrent: isCurrentTab

                    Timer {
                        id: fadeInInterval
                        interval: SB2Launcher_FadeInTimeout

                        onTriggered: {
                            function getAbsPos(field) {
                                var result = 0;
                                var it = tcItem;
                                while (it)
                                {
                                    result += it[field];
                                    it = it.parent;
                                }
                                return result;
                            }
                            var absPoint = quarkProxy.mapToGlobal(getAbsPos("x"), getAbsPos("y"));
                            SB2_launcherProxy.tabListRequested(tabClassID, absPoint.x + rootRect.width, absPoint.y + pregap.height);
                        }
                    }

                    onTriggered: SB2_launcherProxy.tabOpenRequested(tabClassID)
                    onHovered: fadeInInterval.start()
                    onHoverLeft: {
                        fadeInInterval.stop()
                        SB2_launcherProxy.tabListUnhovered(tabClassID)
                    }

                    effect: Colorize {
                        strength: openedTabsCount || tcButton.isHovered ? 0 : 0.3
                        color: "gray"

                        Behavior on strength { PropertyAnimation {} }
                    }

                    ActionButton {
                        id: removeButton

                        visible: canOpenTab && quarkDisplayRoot.settingsMode
                        opacity: 0

                        width: parent.width / 2
                        height: parent.height / 2
                        anchors.top: parent.top
                        anchors.right: parent.right

                        actionIconURL: "image://ThemeIcons/list-remove"
                        transparentStyle: true
                        onTriggered: SB2_launcherProxy.tabClassHideRequested(tabClassID)

                        states: [
                            State {
                                name: "hovered"
                                when: quarkDisplayRoot.settingsMode
                                PropertyChanges { target: removeButton; opacity: 1 }
                            }
                        ]

                        transitions: [
                            Transition {
                                from: ""
                                to: "hovered"
                                reversible: true
                                PropertyAnimation { properties: "opacity"; duration: 200 }
                            }
                        ]
                    }
                }

                Item {
                    id: postgap
                    height: (index != launcherItemRepeater.count - 1 && isCurrentTab) ? rootRect.currentGapSize : 0
                    Behavior on height { PropertyAnimation { duration: 100 } }
                    anchors.bottom: parent.bottom
                }
            }
        }
    }
}
