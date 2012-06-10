import QtQuick 1.0

Rectangle {
    anchors.fill: parent
    smooth: true

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
        width: 100
        height: 100
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

    Text {
        id: shortDescLabel
        text: artistInfo
        textFormat: Text.RichText
        clip: true
        color: "#aaaaaa"
        wrapMode: Text.WordWrap
        anchors.leftMargin: 5
        anchors.left: parent.left
        anchors.rightMargin: 5
        anchors.right: parent.right
        anchors.top: artistImageThumb.bottom
        anchors.topMargin: 5
        anchors.bottom: parent.bottom
    }
}
