import QtQuick 1.0

Rectangle {
    id: rootRect
    anchors.fill: parent
    smooth: true
    radius: 10

    color: "#ee000000"

    signal toggleMountRequested(string id)

    Text {
        id: topLabel

        text: devicesLabelText
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 12
        color: "#eeeeee"

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 5
    }

    ListView {
        id: devicesView

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: topLabel.bottom
        anchors.topMargin: 3
        anchors.bottom: parent.bottom

        clip: true

        model: devModel
        spacing: 20

        delegate: Rectangle {
            color: "#00000000"

            width: devicesView.width
            height: devNameLabel.height + totalSizeLabel.height + mountedAtLabel.height

            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.right: parent.right
                anchors.rightMargin: 5

                height: 1

                color: "#777777"
            }

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
                anchors.topMargin: 2
                anchors.left: parent.left
                anchors.leftMargin: 5
            }

            Text {
                id: mountedAtLabel
                text: mountedAt
                color: "#999999"
                font.italic: true

                visible: text.length != 0

                anchors.top: totalSizeLabel.bottom
                anchors.topMargin: 2
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
                    anchors.margins: -2
                    acceptedButtons: Qt.LeftButton
                    hoverEnabled: true

                    onClicked: rootRect.toggleMountRequested(devID)
                }

                Rectangle {
                    id: mountButtonHover
                    anchors.fill: parent
                    anchors.margins: -1
                    radius: 2

                    visible: mountButtonArea.containsMouse

                    color: "#00000000"
                    border.width: 1
                    border.color: "#888888"
                }
            }
        }
    }
}
