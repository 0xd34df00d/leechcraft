import QtQuick 1.1
import "../common/Common.js" as Common
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
        id: setQuarkOrderButton
        visible: enableSettingsModeButton.settingsMode
        height: width
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: addQuarkButton.top

        actionIconURL: "image://ThemeIcons/format-list-unordered"

        onTriggered: Common.showTooltip(setQuarkOrderButton, function(x, y) { quarkProxy.quarkOrderRequested(x, y) })
    }

    ActionButton {
        id: addQuarkButton

        visible: enableSettingsModeButton.settingsMode
        height: width
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: enableSettingsModeButton.top

        actionIconURL: "image://ThemeIcons/list-add"

        onTriggered: Common.showTooltip(addQuarkButton, function(x, y) { quarkProxy.quarkAddRequested(x, y) })
    }

    ListView {
        id: itemsView

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: setQuarkOrderButton.top

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
                anchors.left: itemsDelegate.left
                anchors.right: itemsDelegate.right
                anchors.leftMargin: 1
                anchors.rightMargin: 1

                clip: true
            }

            Rectangle {
                visible: enableSettingsModeButton.settingsMode
                anchors.fill: itemLoader

                smooth: true
                gradient: Gradient {
                    GradientStop {
                        id: topHighlightGradient
                        position: 0
                        color: colorProxy.setAlpha(colorProxy.color_ToolButton_SelectedTopColor, 0.1)
                    }
                    GradientStop {
                        id: bottomHighlightGradient
                        position: 1
                        color: colorProxy.setAlpha(colorProxy.color_ToolButton_SelectedBottomColor, 0.1)
                    }
                }
                border.color: colorProxy.color_ToolButton_SelectedBorderColor
                border.width: 1
                radius: width / 10

                states: [
                    State {
                        name: "highlight"
                        when: quarkProxy.extHoveredQuarkClass == quarkClass
                        PropertyChanges {
                            target: topHighlightGradient
                            color: colorProxy.setAlpha(colorProxy.color_ToolButton_SelectedTopColor, 0.3)
                        }
                        PropertyChanges {
                            target: bottomHighlightGradient
                            color: colorProxy.setAlpha(colorProxy.color_ToolButton_SelectedBottomColor, 0.3)
                        }
                    }
                ]

                transitions: [
                    Transition {
                        from: ""
                        to: "highlight"
                        reversible: true
                        PropertyAnimation { properties: "color"; duration: 200 }
                    }
                ]
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
        }
    }
}
