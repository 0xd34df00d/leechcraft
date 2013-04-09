import QtQuick 1.1
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real itemSize: parent.quarkBaseSize
    property real length: sensorsView.count * itemSize
    width: viewOrient == "vertical" ? itemSize : length
    height: viewOrient == "vertical" ? length : itemSize

    radius: 2

    color: "transparent"

    ListView {
        id: sensorsView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds

        model: HS_sensorsModel

        orientation: viewOrient == "vertical" ? ListView.Vertical : ListView.Horizontal

        delegate: Image {
            height: rootRect.itemSize
            width: rootRect.itemSize

            sourceSize.width: width
            sourceSize.height: height

            source: iconURL
            asynchronous: true

            Text {
                anchors.centerIn: parent
                text: lastTemp
                font.pixelSize: parent.height / 3
            }
        }
    }
}
