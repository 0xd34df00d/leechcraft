import QtQuick 2.3

Image {
    id: browseInfoImage

    width: 16
    height: 16
    smooth: true
    fillMode: Image.PreserveAspectFit

    source: "image://ThemeIcons/dialog-information"

    cache: false

    signal clicked()

    MouseArea {
        id: browseInfoArea
        anchors.fill: parent
        anchors.margins: -2
        hoverEnabled: true

        onClicked: browseInfoImage.clicked()
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: -1
        radius: 2

        visible: browseInfoArea.containsMouse

        color: "#00000000"
        border.width: 1
        border.color: "#888888"
    }
}
