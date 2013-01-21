import QtQuick 1.0
import org.LC.common 1.0

Rectangle {
    id: rootRect

    width: parent.width
    height: width * launchView.count
    color: "transparent"

    ListView {
        id: launchView

        anchors.fill: parent
        model: Launchy_itemModel

        delegate: ActionButton {
            id: launchViewDelegate

            width: rootRect.width
            height: width

            actionIconURL: "image://LaunchyItemIcons/" + permanentID
            textTooltip: appName

            onTriggered: Launchy_proxy.launch(permanentID)

            ActionButton {
                id: removeButton

                visible: quarkDisplayRoot.settingsMode
                opacity: 0

                width: parent.width / 2
                height: parent.height / 2
                anchors.top: parent.top
                anchors.right: parent.right

                actionIconURL: "image://ThemeIcons/list-remove"
                transparentStyle: true
                onTriggered: Launchy_proxy.remove(permanentID)

                states: [
                    State {
                        name: "settingsMode"
                        when: quarkDisplayRoot.settingsMode
                        PropertyChanges { target: removeButton; opacity: 1 }
                    }
                ]

                transitions: [
                    Transition {
                        from: ""
                        to: "settingsMode"
                        reversible: true
                        PropertyAnimation { properties: "opacity"; duration: 200 }
                    }
                ]
            }

            ListView.onRemove: SequentialAnimation {
                PropertyAction { target: launchViewDelegate; property: "ListView.delayRemove"; value: true }
                NumberAnimation { target: launchViewDelegate; property: "height"; to: 0; duration: 250; easing.type: Easing.InOutQuad }
                PropertyAction { target: launchViewDelegate; property: "ListView.delayRemove"; value: false }
            }
        }
    }
}
