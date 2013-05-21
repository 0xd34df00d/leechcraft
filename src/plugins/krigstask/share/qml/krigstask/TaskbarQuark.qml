import QtQuick 1.1

import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real itemSize: parent.quarkBaseSize
    property real longDim: Math.max(taskbarColumn.rows, taskbarColumn.columns) * itemSize + 2

    width: viewOrient == "vertical" ? itemSize : longDim
    height: viewOrient == "vertical" ? longDim : itemSize

    radius: 2

    smooth: true

    color: "transparent"

    Common { id: commonJS }

    Grid {
        id: taskbarColumn
        anchors.top: parent.top
        anchors.left: parent.left

        columns: viewOrient == "vertical" ? 1 : launcherItemRepeater.count
        rows: viewOrient == "vertical" ? launcherItemRepeater.count : 1

        Repeater {
            id: launcherItemRepeater
            model: KT_appsModel
            Item {
                id: taskbarItem

                height: rootRect.itemSize
                width: rootRect.itemSize

                ActionButton {
                    id: tcButton

                    height: rootRect.itemSize
                    width: rootRect.itemSize

                    actionIconURL: "image://TaskbarIcons/" + windowID + '/' + iconGenID
                    textTooltip: windowName
                    transparentStyle: true

                    isHighlight: !isMinimizedWindow
                    isStrongHighlight: isActiveWindow

                    onTriggered: isActiveWindow ?
                            KT_taskbarProxy.minimizeWindow(windowID) :
                            KT_taskbarProxy.raiseWindow(windowID);
                }
            }
        }
    }
}
