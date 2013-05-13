import QtQuick 1.1
import org.LC.common 1.0
import "lcqml:org/LC/common/Common.js" as Common

Rectangle {
    id: rootRect

    width: parent.quarkBaseSize
    height: parent.quarkBaseSize

    color: "transparent"

    ActionButton {
        id: anButton

        anchors.fill: parent
        actionIconURL: "image://ThemeIcons/preferences-desktop-notification"
        textTooltip: AN_quarkTooltip

        onTriggered: {
            var global = Common.getTooltipPos(anButton);
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
