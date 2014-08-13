import QtQuick 2.3

Row {
    id: rootRow

    signal bookmarkRequested()
    signal previewRequested()
    signal browseInfo()

    spacing: 8

    property bool bookmarkVisible: true
    property bool previewVisible: true

    BrowseButton {
        id: browseInfoImage

        onClicked: rootRow.browseInfo()
    }

    Image {
        id: addToList

        width: 16
        height: 16
        smooth: true
        fillMode: Image.PreserveAspectFit

        source: "image://ThemeIcons/bookmark-new"
        visible: rootRow.bookmarkVisible

        cache: false

        MouseArea {
            id: addToListArea
            anchors.fill: parent
            anchors.margins: -2
            hoverEnabled: true

            onClicked: rootRow.bookmarkRequested()
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: -1
            radius: 2

            visible: addToListArea.containsMouse

            color: "#00000000"
            border.width: 1
            border.color: "#888888"
        }
    }

    PreviewAudioButton {
        id: previewAudio

        visible: rootRow.previewVisible

        onClicked: rootRow.previewRequested()
    }
}
