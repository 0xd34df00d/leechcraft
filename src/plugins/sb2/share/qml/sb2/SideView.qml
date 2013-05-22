import QtQuick 1.1
import org.LC.common 1.0

Rectangle {
    id: quarkDisplayRoot
    anchors.fill: parent

    property alias settingsMode: enableSettingsModeButton.settingsMode
    property bool isVert: viewOrient == "vertical"

    Common { id: commonJS }

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
        width: isVert ? parent.width : parent.height
        height: width
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        actionIconURL: "image://ThemeIcons/preferences-plugin"
        textTooltip: SB2_settingsModeTooltip

        property bool settingsMode: false
        onTriggered: { isHighlight = !isHighlight; settingsMode = !settingsMode; }
    }

    ActionButton {
        id: setQuarkOrderButton
        visible: enableSettingsModeButton.settingsMode
        width: isVert ? parent.width : parent.height
        height: width
        anchors.bottom: isVert ? addQuarkButton.top : undefined
        anchors.right: isVert ? undefined : addQuarkButton.left

        actionIconURL: "image://ThemeIcons/format-list-unordered"
        textTooltip: SB2_quarkOrderTooltip

        onTriggered: commonJS.showTooltip(setQuarkOrderButton, function(x, y) { quarkProxy.quarkOrderRequested(x, y) })
    }

    ActionButton {
        id: addQuarkButton

        visible: enableSettingsModeButton.settingsMode
        width: isVert ? parent.width : parent.height
        height: width
        anchors.bottom: isVert ? enableSettingsModeButton.top : undefined
        anchors.right: isVert ? undefined : enableSettingsModeButton.left

        actionIconURL: "image://ThemeIcons/list-add"
        textTooltip: SB2_addQuarkTooltip

        onTriggered: commonJS.showTooltip(addQuarkButton, function(x, y) { quarkProxy.quarkAddRequested(x, y) })
    }

    ListView {
        id: itemsView

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: isVert ? parent.right : setQuarkOrderButton.left
        anchors.bottom: isVert ? setQuarkOrderButton.top : parent.bottom

        model: itemsModel
        spacing: 2

        orientation: isVert ? ListView.Vertical : ListView.Horizontal

        delegate: Rectangle {
            id: itemsDelegate

            height: isVert ? itemLoader.height : itemsView.height
            width: isVert ? itemsView.width : itemLoader.width

            color: "transparent"

            Loader {
                id: itemLoader

                property real quarkBaseSize: isVert ? width : height

                source: sourceURL
                height: isVert ? item.height : itemsDelegate.height
                width: isVert ? itemsDelegate.width : item.width

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
