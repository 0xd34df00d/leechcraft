import QtQuick 1.1
import "../common/"

Rectangle {
    id: quarkDisplayRoot
    anchors.fill: parent

    property alias settingsMode: enableSettingsModeButton.settingsMode

    gradient: Gradient {
        GradientStop {
            position: 0
            color: colorProxy.color_Panel_TopColor
        }
        GradientStop {
            position: 1
            color: colorProxy.color_Panel_BottomColor
        }
    }

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

    ActionButton {
        id: addQuarkButton

        visible: enableSettingsModeButton.settingsMode
        opacity: 0
        z: 11

        actionIconURL: "image://ThemeIcons/list-add"

        onTriggered: quarkProxy.quarkAddRequested()
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
                transparentStyle: true

                property real dimension: Math.min(itemLoader.width / 2, itemLoader.height / 2)
                width: dimension
                height: dimension
                anchors.bottom: itemLoader.bottom
                anchors.right: itemLoader.right

                states: [
                    State {
                        name: "inSettingsMode"
                        when: enableSettingsModeButton.settingsMode
                        PropertyChanges { target: settingsButton; opacity: 1 }
                    }
                ]

                transitions: [
                    Transition {
                        from: ""
                        to: "inSettingsMode"
                        reversible: true
                        PropertyAnimation { properties: "opacity"; duration: 200 }
                    }
                ]

                onTriggered: quarkProxy.showSettings(sourceURL)
            }

            ActionButton {
                id: removeQuarkButton

                visible: enableSettingsModeButton.settingsMode
                opacity: 0
                z: 11

                actionIconURL: "image://ThemeIcons/edit-delete"
                transparentStyle: true

                anchors.top: itemLoader.top
                anchors.left: itemLoader.left
                anchors.right: itemLoader.right
                height: width

                states: [
                    State {
                        name: "inSettingsMode"
                        when: enableSettingsModeButton.settingsMode
                        PropertyChanges { target: removeQuarkButton; opacity: 1 }
                        PropertyChanges { target: itemsDelegate; height: Math.max(itemLoader.item.height, itemsDelegate.width) }
                    }
                ]

                transitions: [
                    Transition {
                        from: ""
                        to: "inSettingsMode"
                        reversible: true
                        PropertyAnimation { properties: "opacity,height"; duration: 200 }
                    }
                ]

                onTriggered: quarkProxy.removeQuark(sourceURL)
            }
        }
    }
}
