import QtQuick 1.0

Rectangle {
    id: imgView

    signal imageSelected(string imageId)

    Component {
        id: imageListDelegate

        Item {
            width: imagesGrid.cellWidth - 2
            height: imagesGrid.cellHeight - 2
            clip: true

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

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton

                    onClicked: imgView.imageSelected(image)
                }
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
        anchors.rightMargin: 5
        anchors.leftMargin: 5
        anchors.bottomMargin: 2
        anchors.topMargin: 2

        anchors.fill: parent
        cellWidth: 100
        cellHeight: 100

        focus: true
        delegate: imageListDelegate
        model: filesListModel
    }

    Scrollbar {
        id: verScrollBar
        width: 5
        height: parent.height - 12
        anchors.left: imagesGrid.right
        orientation: Qt.Vertical
        position: imagesGrid.visibleArea.yPosition
        pgSize: imagesGrid.visibleArea.heightRatio
    }
}
