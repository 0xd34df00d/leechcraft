import QtQuick 1.0

Rectangle
{
    gradient: Gradient {
        GradientStop {
            position: 0
            color: "#ff6600"
        }

        GradientStop {
            position: 1
            color: "#ff1d00"
        }
    }
    anchors.fill: parent

    GridView {
        anchors.fill: parent
        id: releasesView

        model: releasesModel
        cellHeight: 180
        cellWidth: 300

        delegate: Item {
            height: releasesView.cellHeight
            width: releasesView.cellWidth

            Rectangle {
                id: delegateRect
                anchors.fill: parent
                anchors.margins: 10
                radius: 5
                gradient: Gradient {
                    GradientStop {
                        position: 1
                        color: "#42394b"
                    }

                    GradientStop {
                        position: 0
                        color: "#000000"
                    }
                }

                border.width: 0
                border.color: "#000000"
                smooth: true

                Column {
                    id: column1
                    anchors.fill: parent
                    anchors.topMargin: 5

                    Image {
                        id: albumImageThumb
                        width: 100
                        height: 100
                        smooth: true
                        fillMode: Image.PreserveAspectFit
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: albumThumbImage
                    }

                    Text {
                        id: albumNameLabel
                        text: albumName
                        horizontalAlignment: Text.AlignHCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                        elide: Text.ElideMiddle
                        font.bold: true
                        font.pointSize: 12
                        color: "#dddddd"
                    }

                    Text {
                        id: artistLabel
                        text: artistName
                        elide: Text.ElideMiddle
                        color: "#999999"
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.pointSize: 8
                    }

                    Text {
                        id: dateLabel
                        text: releaseDate
                        color: "#888888"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }
        }
    }
}
