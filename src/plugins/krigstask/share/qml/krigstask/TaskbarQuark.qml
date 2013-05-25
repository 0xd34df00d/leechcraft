import QtQuick 1.1

import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real itemSize: parent.quarkBaseSize
    property real longDim: Math.max(taskbarColumn.rows, taskbarColumn.columns) * itemSize + 2

    property bool isExpandable: true

    implicitWidth: viewOrient == "vertical" ? itemSize : longDim
    implicitHeight: viewOrient == "vertical" ? longDim : itemSize

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
                width: viewOrient == "vertical" ?
                        rootRect.itemSize :
                        Math.min(150, rootRect.width / launcherItemRepeater.count)

                ActionButton {
                    id: tcButton

                    anchors.fill: parent

                    actionIconURL: "image://TaskbarIcons/" + windowID + '/' + iconGenID
                    textTooltip: windowName
                    decoOpacity: isMinimizedWindow ? 0.3 : (isActiveWindow ? 0.8 : 0.5)
                    orientation: viewOrient
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    isHighlight: !isMinimizedWindow
                    isStrongHighlight: isActiveWindow

                    onTriggered: isActiveWindow ?
                            KT_taskbarProxy.minimizeWindow(windowID) :
                            KT_taskbarProxy.raiseWindow(windowID);
                    onClicked: commonJS.showTooltip(tcButton,
                            function(x, y) { KT_taskbarProxy.showMenu(windowID, x, y) })

                    actionText: windowName

                    ActionButton {
                        opacity: viewOrient == "horizontal" && (parent.isHovered || isHovered) ? 1 : 0
                        Behavior on opacity { PropertyAnimation {} }

                        anchors.right: parent.right
                        anchors.rightMargin: parent.height / 4
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.height / 2
                        height: parent.height / 2

                        actionIconURL: "image://ThemeIcons/window-close"
                        onTriggered: KT_taskbarProxy.closeWindow(windowID)
                    }
                }
            }
        }
    }
}
