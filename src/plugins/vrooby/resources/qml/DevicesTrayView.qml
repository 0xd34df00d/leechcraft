import QtQuick 1.0

Rectangle {
    id: rootRect
    anchors.fill: parent
    smooth: true
    radius: 10

    color: "#dd000000"

    signal toggleMountRequested(string id)

    ListView {
        id: devicesView

        anchors.fill: parent
        model: devModel

        spacing: 10

        delegate: Rectangle {
            color: "#00000000"

            width: devicesView.width
            height: 40

            Text {
                id: devNameLabel
                text: devName
                color: "#dddddd"
                font.bold: true

                anchors.top: parent.top
                anchors.topMargin: 5
                anchors.left: parent.left
                anchors.leftMargin: 5
            }

            Text {
                id: devFileLabel
                text: devFile
                color: "#888888"
                font.italic: true

                anchors.top: parent.top
                anchors.topMargin: 5
                anchors.left: parent.left
                anchors.leftMargin: parent.width - 60
                anchors.right: parent.right
                anchors.rightMargin: 5
            }

            Text {
                id: totalSizeLabel
                text: formattedTotalSize
                color: "#aaaaaa"
                font.italic: true

                anchors.top: devNameLabel.bottom
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 5
            }

            Text {
                id: mountedAtLabel
                text: mountedAt
                color: "#999999"
                font.italic: true

                visible: text.length != 0

                anchors.top: totalSizeLabel.bottom
                anchors.topMargin: 5
                anchors.left: parent.left
                anchors.leftMargin: 5
            }

            Image {
                id: mountButton
                source: mountButtonIcon
                width: 22
                height: 22
                smooth: true

                anchors.top: devFileLabel.bottom
                anchors.topMargin: 5
                anchors.right: parent.right
                anchors.rightMargin: 5

                MouseArea {
                    id: mountButtonArea
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton

                    onClicked: rootRect.toggleMountRequested(devID)
                }
            }
        }
    }
}
