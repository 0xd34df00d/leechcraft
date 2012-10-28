import QtQuick 1.1
import "."

Rectangle {
    id: rootRect

    width: parent.width
    height: tasksView.count * 10

    ListView {
        id: tasksView

        anchors.fill: parent
        interactive: false

        model: TPI_infoModel

        delegate: Rectangle {
            width: parent.width
            height: 10

            border.color: "#777777"
            border.width: 1

            ProgressBar {
                minimum: 0
                maximum: jobTotal
                value: jobDone

                color: "#999999"

                anchors.fill: parent
            }
        }
    }
}
