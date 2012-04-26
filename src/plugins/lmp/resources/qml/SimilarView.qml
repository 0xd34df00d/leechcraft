import QtQuick 1.0

ListView {
    anchors.fill: parent
    id: similarView
    boundsBehavior: Flickable.StopAtBounds

    model: similarModel
    delegate: Rectangle {
        id: delegateRect
        height: 150
        radius: 5
        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#aa444c80"
            }

            GradientStop {
                position: 1
                color: "#aa001080"
            }
        }
        border.width: 0
        border.color: "#000000"
        smooth: true
        width: similarView.width
        Text {
            id: artistNameLabel
            text: artistName
            font.bold: true
            font.pointSize: 12
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
        }

        Text {
            id: similarityLabel
            text: similarity
            anchors.top: parent.top
            anchors.topMargin: 2
            anchors.right: parent.right
            anchors.rightMargin: 2
        }

        Text {
            id: artistTagsLabel
            text: artistTags
            anchors.leftMargin: 5
            anchors.left: artistImageThumb.right
            anchors.top: artistNameLabel.bottom
            anchors.topMargin: 0
            font.pointSize: 8
        }

        Text {
            id: shortDescLabel
            text: shortDesc
            width: parent.width - artistImageThumb.width - 10
            clip: true
            wrapMode: Text.WordWrap
            anchors.leftMargin: 5
            anchors.left: artistImageThumb.right
            anchors.top: artistTagsLabel.bottom
            anchors.topMargin: 0
            anchors.bottom: parent.bottom
        }
    }
}
