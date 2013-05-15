import QtQuick 1.1
import Effects 1.0
import SB2 1.0
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real itemSize: parent.quarkBaseSize
    property real longDim: Math.max(launcherColumn.rows, launcherColumn.columns) * itemSize + 2 + (addTCButton.visible ? addTCButton.height : 0)

    width: viewOrient == "vertical" ? itemSize : longDim
    height: viewOrient == "vertical" ? longDim : itemSize

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

                Rectangle {
                    id: highlightOpenedRect

                    opacity: openedTabsCount ? 1 : 0
                    Behavior on opacity { PropertyAnimation {} }

                    width: viewOrient == "vertical" ? 2 : 12
                    height: viewOrient == "vertical" ? 12 : 2
                    radius: 1

                    anchors.bottom: viewOrient == "vertical" ? undefined : tcButton.bottom
                    anchors.horizontalCenter: viewOrient == "vertical" ? undefined : tcButton.horizontalCenter
                    anchors.left: viewOrient == "vertical" ? tcButton.left : undefined
                    anchors.verticalCenter: viewOrient == "vertical" ? tcButton.verticalCenter : undefined

                    color: colorProxy.color_ToolButton_SelectedBorderColor
                }

                Rectangle {
                    id: highlightCurrentRect

                    opacity: isCurrentTab ? 1 : 0
                    Behavior on opacity { PropertyAnimation {} }

                    width: viewOrient == "vertical" ? 2 : 12
                    height: viewOrient == "vertical" ? 12 : 2
                    radius: 1

                    anchors.top: viewOrient == "vertical" ? undefined : tcButton.top
                    anchors.horizontalCenter: viewOrient == "vertical" ? undefined : tcButton.horizontalCenter
                    anchors.right: viewOrient == "vertical" ? tcButton.right : undefined
                    anchors.verticalCenter: viewOrient == "vertical" ? tcButton.verticalCenter : undefined

                    color: colorProxy.color_ToolButton_SelectedBorderColor
                }

                ActionButton {
                    id: tcButton

                    height: rootRect.itemSize
                    width: rootRect.itemSize

                    actionIconURL: tabClassIcon
                    textTooltip: tabClassName
                    isHighlight: false
                    transparentStyle: true
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

                    effect: Desaturate {
                        strength: openedTabsCount || tcButton.isHovered ? 0 : 0.5

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
            }
        }
    }
}
