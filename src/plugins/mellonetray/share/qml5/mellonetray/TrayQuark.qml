import QtQuick 2.3

import org.LC.common 1.0
import Mellonetray 1.0

Rectangle {
    id: rootRect

    property real itemSize: parent.quarkBaseSize
    property real longDim: Math.max(trayColumn.rows, trayColumn.columns) * itemSize + 2

    implicitWidth: viewOrient == "vertical" ? itemSize : longDim
    implicitHeight: viewOrient == "vertical" ? longDim : itemSize

    radius: 2

    smooth: true

    color: "transparent"

    Grid {
        id: trayColumn
        anchors.top: parent.top
        anchors.left: parent.left

        columns: viewOrient == "vertical" ? 1 : launcherItemRepeater.count
        rows: viewOrient == "vertical" ? launcherItemRepeater.count : 1

        Repeater {
            id: launcherItemRepeater
            model: MT_trayModel
            Item {
                id: trayItem

                width: rootRect.itemSize
                height: rootRect.itemSize

                IconHandler {
                    id: trayIcon

                    anchors.fill: parent
                    wid: itemID
                }
            }
        }
    }
}
