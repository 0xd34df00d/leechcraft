import QtQuick 1.1

Image {
    id: previewAudio

    width: 16
    height: 16
    smooth: true
    fillMode: Image.PreserveAspectFit

    source: "image://sysIcons/media-playback-start"

    signal clicked()

    MouseArea {
        id: previewAudioArea
        anchors.fill: parent
        anchors.margins: -2
        hoverEnabled: true
        onClicked: previewAudio.clicked()
    }

    Rectangle {
        id: previewAudioHover
        anchors.fill: parent
        anchors.margins: -1
        radius: 2

        visible: previewAudioArea.containsMouse

        color: "#00000000"
        border.width: 1
        border.color: "#888888"
    }
}
