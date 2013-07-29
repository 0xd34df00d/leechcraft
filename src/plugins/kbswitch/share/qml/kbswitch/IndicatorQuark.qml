import QtQuick 1.1
import org.LC.common 1.0

Item {
    id: rootRect

    implicitWidth: parent.quarkBaseSize
    implicitHeight: parent.quarkBaseSize

    Image {
        anchors.fill: parent
    }

    Text {
        anchors.fill: parent

        font.pixelSize: height / 2
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        color: colorProxy.color_Panel_TextColor

        text: KBSwitch_proxy.currentLangCode
    }
}
