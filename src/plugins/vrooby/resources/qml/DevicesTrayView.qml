import QtQuick 1.0

Rectangle {
    anchors.fill: parent
    smooth: true
    radius: 10

    color: "#dd000000"

    ListView {
        id: devicesView

        anchors.fill: parent
        model: devModel

        spacing: 10

        delegate: Rectangle {
            color: "#00000000"

            width: devicesView.width
            height: 30

            Text {
                id: devNameLabel
                text: devName
                color: "#dddddd"
                font.bold: true

                anchors.top: parent.top
                anchors.topMargin: 5
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 5
            }

            Text {
                text: devFile
                color: "#999999"
                font.italic: true

                anchors.top: parent.top
                anchors.topMargin: 5
                anchors.left: parent.left
                anchors.leftMargin: parent.width - 60
                anchors.right: parent.right
                anchors.rightMargin: 5
            }
        }
    }
}
