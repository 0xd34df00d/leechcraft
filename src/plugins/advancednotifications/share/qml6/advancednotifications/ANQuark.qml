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
            const params = {
                rulesManager: AN_rulesManager,
                proxy: AN_proxy
            };
            commonJS.toggleTooltip(anButton, params, Qt.resolvedUrl("RulesListView.qml"));
        }
    }
}
