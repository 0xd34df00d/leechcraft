import QtQuick 1.1

Rectangle {
    id: imgView

    Keys.onEscapePressed: { imgView.state = '' }

    signal imageSelected(string imageId)

    function showSingleImage(url) {
        singleImage.source = url
        imgView.state = 'singleImageMode'
    }

    SystemPalette {
        id: sysPalette
        colorGroup: SystemPalette.Active
    }

    color: sysPalette.window

    Component {
        id: imageListDelegate

        Item {
            width: imagesGrid.cellWidth
            height: imagesGrid.cellHeight

            Item {
                anchors.fill: parent
                anchors.topMargin: parent.height * 0.05
                anchors.bottomMargin: parent.height * 0.05
                anchors.leftMargin: parent.width * 0.05
                anchors.rightMargin: parent.width * 0.05
                clip: true

                Image {
                    id: theImage

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: nameLabel.top
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    asynchronous: true

                    source: image

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton

                        onClicked: imgView.imageSelected(image)
                    }
                }

                Text {
                    id: nameLabel

                    anchors.bottom: filesizeLabel.top
                    anchors.left: parent.left
                    width: imagesGrid.cellWidth * 0.9

                    text: filename
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter

                    textFormat: Text.PlainText
                }

                Text {
                    id: filesizeLabel

                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width

                    text: filesize
                    font.italic: true
                    horizontalAlignment: Text.AlignHCenter
                }
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

            NumberAnimation { target: imagesGridContainer; property: "opacity"; to: 0.1; duration: 300 }
            NumberAnimation { target: singleImageContainer; property: "opacity"; to: 1; duration: 120 }
        }
    ]
}
