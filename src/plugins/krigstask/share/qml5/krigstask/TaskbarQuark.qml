import QtQuick 2.3
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real itemSize: parent.quarkBaseSize
    property real longDim: Math.max(taskbarColumn.rows, taskbarColumn.columns) * itemSize + 2

    property bool isExpandable: true

    implicitWidth: viewOrient == "vertical" ? itemSize : longDim + itemSize
    implicitHeight: viewOrient == "vertical" ? longDim + itemSize : itemSize

    radius: 2

    smooth: true

    color: "transparent"

    Common { id: commonJS }

    Grid {
        id: taskbarColumn
        anchors.top: parent.top
        anchors.left: parent.left

        width: viewOrient == "vertical" ? rootRect.itemSize : longDim
        height: viewOrient == "vertical" ? longDim : rootRect.itemSize

        function calcCount() {
            var cnt = 0;
            for (var i = 0; i < launcherItemRepeater.count; ++i) {
                var item = launcherItemRepeater.itemAt(i);
                if (item && item.visible)
                    ++cnt;
            }
            cnt += showPager ? 1 : 0;
            return cnt;
        }

        property alias fullItemCount: launcherItemRepeater.count

        columns: viewOrient == "vertical" ? taskbarRowCount : fullItemCount
        rows: viewOrient == "vertical" ? fullItemCount : taskbarRowCount

        flow: viewOrient == "vertical" ? Grid.LeftToRight : Grid.TopToBottom

        property int itemHeight: rootRect.itemSize / taskbarRowCount
        property int itemWidth: viewOrient == "vertical" ?
                            rootRect.itemSize / taskbarRowCount :
                            Math.min(150, rootRect.width * taskbarRowCount / taskbarColumn.calcCount())

        Repeater {
            id: launcherItemRepeater
            model: KT_appsModel
            Item {
                id: taskbarItem

                visible: showFromAllDesks || isCurrentDesktop

                height: taskbarColumn.itemHeight
                width: taskbarColumn.itemWidth

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

    ActionButton {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        id: showPagerButton

        width: rootRect.itemSize
        height: rootRect.itemSize

        visible: showPager

        actionIconURL: "image://ThemeIcons/user-desktop"
        onTriggered: commonJS.showTooltip(showPagerButton,
                function(x, y) { KT_taskbarProxy.showPager(x, y, showThumbsInPager) })
    }
}
