import QtQuick 2.3
import org.LC.common 1.0
import org.LC.Ooronee 1.0

Rectangle {
    id: rootRect
    implicitWidth: parent.quarkBaseSize
    implicitHeight: parent.quarkBaseSize

    color: "transparent"

    border.color: "gray"
    border.width: 1
    radius: width / 8

    property variant enterTime: null

    Image {
        anchors.fill: parent
        source: "image://ThemeIcons/edit-paste"
    }

    DropArea {
        anchors.fill: parent

        onDataDropped: {
            var now = new Date();
            Ooronee_Proxy.handle(data, (now - enterTime) > Ooronee_Proxy.hoverTimeout);
        }

        onDragEntered: enterTime = new Date();
    }
}
