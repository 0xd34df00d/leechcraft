import QtQuick 1.0

Rectangle {
    id: imgView

    Keys.onEscapePressed: { imgView.state = '' }

    signal imageSelected(string imageId)

    function showSingleImage(url) {
        singleImage.source = url
        imgView.state = 'singleImageMode'
    }

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

    Item {
        id: imagesGridContainer
        anchors.fill: parent

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

    Item {
        id: singleImageContainer

        anchors.fill: parent
        visible: false
        opacity: 0

        Image {
            id: singleImage

            fillMode: Image.PreserveAspectFit
            smooth: true
            clip: true
            asynchronous: true

            anchors.fill: parent
        }
    }

    states: [
        State {
            name: "singleImageMode"

            PropertyChanges {
                target: imagesGridContainer
                opacity: 0.1
            }

            PropertyChanges {
                target: singleImageContainer
                visible: true
                opacity: 1
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "singleImageMode"

            NumberAnimation { target: imagesGridContainer; property: "opacity"; to: 0.1; duration: 500 }
            NumberAnimation { target: singleImageContainer; property: "opacity"; to: 1; duration: 300 }
        }
    ]
}
