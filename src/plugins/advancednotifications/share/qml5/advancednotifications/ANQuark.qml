import QtQuick 2.3
import org.LC.common 1.0

Rectangle {
    id: rootRect

    implicitWidth: parent.quarkBaseSize
    implicitHeight: parent.quarkBaseSize

    color: "transparent"

    Common { id: commonJS }

    ActionButton {
        id: anButton

        anchors.fill: parent
        actionIconURL: "image://ThemeIcons/preferences-desktop-notification"
        textTooltip: AN_quarkTooltip

        onTriggered: {
            var global = commonJS.getTooltipPos(anButton);
            var params = {
                x: global.x,
                y: global.y,
                existing: "toggle",
                "rulesManager": AN_rulesManager,
                "proxy": AN_proxy
            };
            quarkProxy.openWindow(sourceURL, "RulesListView.qml", params);
        }
    }
}
