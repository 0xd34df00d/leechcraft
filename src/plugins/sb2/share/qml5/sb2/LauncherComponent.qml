import QtQuick 2.3
import SB2 1.0
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real itemSize: parent.quarkBaseSize
    property real longDim: Math.max(launcherColumn.rows, launcherColumn.columns) * itemSize + 2 + (addTCButton.visible ? addTCButton.height : 0)

    implicitWidth: viewOrient == "vertical" ? itemSize : longDim
    implicitHeight: viewOrient == "vertical" ? longDim : itemSize

    radius: 2

    smooth: true

    color: "transparent"

    Common { id: commonJS }

    ActionButton {
        id: addTCButton
        visible: quarkDisplayRoot.settingsMode
        anchors.bottom: viewOrient == "vertical" ? parent.bottom : undefined
        anchors.right: viewOrient == "vertical" ? undefined : parent.right
        anchors.horizontalCenter: viewOrient == "vertical" ? parent.horizontalCenter : undefined
        anchors.verticalCenter: viewOrient == "vertical" ? undefined : parent.verticalCenter
        width: parent.itemSize * 2 / 3
        height: parent.itemSize * 2 / 3

        actionIconURL: "image://ThemeIcons/list-add"

        onTriggered: commonJS.showTooltip(addTCButton, function(x, y) { SB2_launcherProxy.tabUnhideListRequested(x, y) })

        LauncherDropArea {
            id: dropArea
            anchors.fill: parent
            onTabDropped: SB2_launcherProxy.tabClassUnhideRequested(tabClass)
        }
    }

    Grid {
        id: launcherColumn
        anchors.top: parent.top
        anchors.left: parent.left

        columns: viewOrient == "vertical" ? 1 : launcherItemRepeater.count
        rows: viewOrient == "vertical" ? launcherItemRepeater.count : 1

        Repeater {
            id: launcherItemRepeater
            model: SB2_launcherModel
            Item {
                id: tcItem

                height: rootRect.itemSize
                width: rootRect.itemSize

                ActionButton {
                    id: tcButton

                    height: rootRect.itemSize
                    width: rootRect.itemSize

                    actionIconURL: tabClassIcon
                    textTooltip: tabClassName
                    isHighlight: openedTabsCount
                    isStrongHighlight: isCurrentTab
                    isCurrent: isCurrentTab

                    Timer {
                        id: fadeInInterval
                        interval: SB2Launcher_FadeInTimeout

                        onTriggered: commonJS.showTooltip(tcButton, function(x, y) { SB2_launcherProxy.tabListRequested(tabClassID, x, y) })
                    }

                    onTriggered: SB2_launcherProxy.tabOpenRequested(tabClassID)
                    onHovered: fadeInInterval.start()
                    onHoverLeft: {
                        fadeInInterval.stop()
                        SB2_launcherProxy.tabListUnhovered(tabClassID)
                    }

                    /*
                    effect: Desaturate {
                        strength: openedTabsCount || tcButton.isHovered ? 0 : 0.5

                        Behavior on strength { PropertyAnimation {} }
                    }
                    */

                    ActionButton {
                        id: removeButton

                        visible: canOpenTab && quarkDisplayRoot.settingsMode
                        opacity: 0

                        width: parent.width / 2
                        height: parent.height / 2
                        anchors.top: parent.top
                        anchors.right: parent.right

                        actionIconURL: "image://ThemeIcons/list-remove"
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
            }
        }
    }
}
