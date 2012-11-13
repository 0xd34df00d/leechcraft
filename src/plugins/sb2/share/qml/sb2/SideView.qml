import QtQuick 1.1

Rectangle {
    id: quarkDisplayRoot
    color: "black"
    anchors.fill: parent

    property alias settingsMode: enableSettingsModeButton.settingsMode

    ActionButton {
        id: enableSettingsModeButton
        height: width
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        actionIconURL: "image://ThemeIcons/preferences-plugin"

        property bool settingsMode: false
        onTriggered: { isHighlight = !isHighlight; settingsMode = !settingsMode; }
    }

    ListView {
        id: itemsView

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: enableSettingsModeButton.top

        model: itemsModel
        spacing: 2

        delegate: Rectangle {
            id: itemsDelegate

            height: itemLoader.height
            width: itemsView.width

            color: "transparent"

            Loader {
                id: itemLoader

                source: sourceURL
                height: item.height
                width: parent.width

                clip: true
            }

            ActionButton {
                id: settingsButton

                visible: quarkHasSettings
                opacity: 0
                z: 10

                actionIconURL: "image://ThemeIcons/preferences-desktop"

                property real dimension: Math.min(itemLoader.width / 2, itemLoader.height / 2)
                width: dimension
                height: dimension
                anchors.bottom: itemLoader.bottom
                anchors.right: itemLoader.right

                states: [
                    State {
                        name: "hovered"
                        when: enableSettingsModeButton.settingsMode
                        PropertyChanges { target: settingsButton; opacity: 1 }
                    }
                ]

                transitions: [
                    Transition {
                        from: ""
                        to: "hovered"
                        reversible: true
                        PropertyAnimation { properties: "opacity"; duration: 200 }
                    }
                ]

                onTriggered: quarkProxy.showSettings(sourceURL)
            }
        }
    }
}
