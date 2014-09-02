import QtQuick 2.3
import org.LC.common 1.0

Item {
    id: rootRect

    implicitWidth: parent.quarkBaseSize
    implicitHeight: parent.quarkBaseSize

    Image {
        anchors.fill: parent
        anchors.margins: parent.width / 10

        source: "image://KBSwitch_flags/" + KBSwitch_proxy.currentLangCode

        smooth: true
        fillMode: Image.PreserveAspectFit
    }

    Text {
        anchors.fill: parent

        font.pixelSize: height / 2
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        color: colorProxy.color_Panel_TextColor

        text: KBSwitch_proxy.currentLangCode

        style: Text.Outline
        styleColor: colorProxy.color_Panel_TopColor
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onPressed: {
            if (pressedButtons & Qt.LeftButton)
                KBSwitch_proxy.setNextLanguage();
            else
                KBSwitch_proxy.contextMenuRequested();
        }
   }
}
