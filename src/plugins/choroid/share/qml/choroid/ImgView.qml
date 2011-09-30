import QtQuick 1.0

Rectangle {
	id: imgView
    width: 800
    height: 600

	Component {
		id: imageListDelegate

		Item {
            width: imagesGrid.cellWidth
            height: imagesGrid.cellHeight

            Image {
                id: theImage

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                fillMode: Image.PreserveAspectFit
                smooth: true

                source: image
            }

            Text {
                id: textFilename
                anchors { top: theImage.bottom; horizontalCenter: parent.horizontalCenter }
                text: filename
            }

            Text {
                anchors { top: textFilename.bottom; horizontalCenter: parent.horizontalCenter }
                text: filesize
            }
		}
	}

    GridView {
        id: imagesGrid

        anchors.fill: parent
        cellWidth: 100
        cellHeight: 100

        focus: true
        delegate: imageListDelegate
        model: filesListModel
    }
}
