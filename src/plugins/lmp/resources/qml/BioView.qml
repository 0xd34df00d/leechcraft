import QtQuick 1.0

Rectangle {
    anchors.fill: parent
    smooth: true

    color: "black"

    Text {
        id: artistNameLabel
        text: artistName
        font.bold: true
        font.pointSize: 12
        color: "#dddddd"
        anchors.top: parent.top
        anchors.topMargin: 2
        anchors.left: artistImageThumb.right
        anchors.leftMargin: 5
    }

    Image {
        id: artistImageThumb
        width: 170
        height: 170
        smooth: true
        fillMode: Image.PreserveAspectFit
        anchors.left: parent.left
        anchors.leftMargin: 2
        anchors.top: parent.top
        anchors.topMargin: 2
        source: artistImageURL

        /*
        MouseArea {
            anchors.fill: parent

            onClicked: {
                fullSizeArtistImg.source = artistBigImageURL
                if (fullSizeArtistImg.status == Image.Ready) fullSizeArtistImg.state = "visible"
            }
        }
        */
    }

    Text {
        id: artistTagsLabel
        text: artistTags
        color: "#999999"
        anchors.leftMargin: 5
        anchors.left: artistImageThumb.right
        anchors.top: artistNameLabel.bottom
        anchors.topMargin: 0
        font.pointSize: 8
    }

    Rectangle {
        id: upTextShade
        z: 2
        height: 10

        anchors.top: artistImageThumb.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        gradient: Gradient {
            GradientStop {
                position: 1
                color: "#00000000"
            }

            GradientStop {
                position: 0
                color: "#ff000000"
            }
        }
    }

    Flickable {
        anchors.leftMargin: 5
        anchors.left: parent.left
        anchors.rightMargin: 5
        anchors.right: parent.right
        anchors.top: artistImageThumb.bottom
        anchors.bottom: parent.bottom

        contentWidth: width
        contentHeight: shortDescLabel.height + 16

        clip: true

        Text {
            id: shortDescLabel
            text: artistInfo
            textFormat: Text.RichText
            clip: true
            color: "#aaaaaa"
            wrapMode: Text.WordWrap

            anchors.top: parent.top
            anchors.topMargin: 8
            anchors.left: parent.left
            anchors.right: parent.right
        }
    }

    Rectangle {
        id: downTextShade
        z: 2
        height: 10

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#00000000"
            }

            GradientStop {
                position: 1
                color: "#ff000000"
            }
        }
    }
}
