import QtQuick 1.1
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

    Image {
        anchors.fill: parent
        source: "image://ThemeIcons/edit-paste"
    }

    Timer {
        id: startTimer

        interval: Ooronee_Proxy.hoverTimeout

        property variant data
        property bool hasTriggered: false

        onTriggered: {
            hasTriggered = true;
            Ooronee_Proxy.handle(data, true);
        }
    }

    DropArea {
        anchors.fill: parent

        onDataDropped: {
            if (startTimer.hasTriggered) {
                startTimer.hasTriggered = false;
                return;
            }

            Ooronee_Proxy.handle(data, false);
        }

        onDragEntered: {
            startTimer.data = data
            startTimer.start()
        }
        onDragLeft: {
            startTimer.hasTriggered = false;
            startTimer.stop()
        }
    }
}
