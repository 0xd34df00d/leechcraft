import QtQuick 2.3
import QtQuick.Controls 1.4

Rectangle {
    id: imgView

    Keys.onPressed: {
        switch (event.key)
        {
        case Qt.Key_PageDown:
        case Qt.Key_Space:
        case Qt.Key_Right:
            console.log("next");
            imgView.nextImageRequested();
            event.accepted = true;
            break;
        case Qt.Key_Left:
            imgView.prevImageRequested();
            event.accepted = true;
            break;
        case Qt.Key_Escape:
            imgView.state = '';
            event.accepted = true;
            break;
        }
    }

    signal imageSelected(string imageId)
    signal nextImageRequested()
    signal prevImageRequested()

    function showSingleImage(url) {
        singleImage.source = url;
        imgView.state = url != '' ? 'singleImageMode' : '';
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

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton

                    onClicked: imgView.imageSelected(image)
                }
            }
        }
    }

    Item {
        id: imagesGridContainer
        anchors.fill: parent

        ScrollView {
            id: flickableBioText

            anchors.fill: parent
            anchors.margins: 2

            GridView {
                id: imagesGrid
                clip: true
                smooth: true

                cellWidth: 100
                cellHeight: 100

                focus: true
                delegate: imageListDelegate
                model: filesListModel
            }
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
