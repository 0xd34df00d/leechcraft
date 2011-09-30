import QtQuick 1.0

Rectangle {
    id: imgView

    Component {
        id: imageListDelegate

        Item {
            width: imagesGrid.cellWidth
            height: imagesGrid.cellHeight
            clip: true
            anchors.rightMargin: 2
            anchors.leftMargin: 2
            anchors.bottomMargin: 2
            anchors.topMargin: 2

            Image {
                id: theImage

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                fillMode: Image.PreserveAspectFit
                smooth: true
                clip: true
                asynchronous: true

                source: image
            }

            Text {
                id: textFilename

                anchors { top: theImage.bottom; horizontalCenter: parent.horizontalCenter }
                width: parent.width

                text: filename
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                id: textFilesize

                anchors { top: textFilename.bottom; horizontalCenter: parent.horizontalCenter; bottom: parent.bottom }
                width: parent.width

                text: filesize
                font.italic: true
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    GridView {
        id: imagesGrid
        clip: true
        smooth: true
        anchors.rightMargin: 2
        anchors.leftMargin: 2
        anchors.bottomMargin: 2
        anchors.topMargin: 2

        anchors.fill: parent
        cellWidth: 100
        cellHeight: 100

        focus: true
        delegate: imageListDelegate
        model: filesListModel
    }
}
