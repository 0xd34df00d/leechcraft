import QtQuick 2.3
import QtQuick.Layouts 1.2
import org.LC.common 1.0

Rectangle {
    id: quarkDisplayRoot

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

    states: [
        State {
            name: "vertState"
            when: isVert

            AnchorChanges {
                target: itemsView
                anchors.right: undefined
                anchors.bottom: ownStuffGrid.top
            }
        },
        State {
            name: "horizState"
            when: !isVert

            AnchorChanges {
                target: itemsView
                anchors.right: ownStuffGrid.left
                anchors.bottom: undefined
            }
        }
    ]

    state: "vertState"

    Grid {
        id: ownStuffGrid

        anchors.right: parent.right
        anchors.bottom: parent.bottom

        columns: isVert ? 1 : 5
        rows: isVert ? 5 : 1

        Item {
            id: setPanelPosWidget

            visible: enableSettingsModeButton.settingsMode
            width: isVert ? quarkDisplayRoot.width : quarkDisplayRoot.height
            height: width

            ActionButton {
                id: moveToTop

                width: parent.width / 3
                height: width
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                hoverScalesIcons: false

                actionIconURL: "image://ThemeIcons/arrow-up"

                onTriggered: quarkProxy.panelMoveRequested("top")
            }

            ActionButton {
                id: moveToBottom

                width: parent.width / 3
                height: width
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                hoverScalesIcons: false

                actionIconURL: "image://ThemeIcons/arrow-down"

                onTriggered: quarkProxy.panelMoveRequested("bottom")
            }

            ActionButton {
                id: moveToLeft

                width: parent.width / 3
                height: width
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                hoverScalesIcons: false

                actionIconURL: "image://ThemeIcons/arrow-left"

                onTriggered: quarkProxy.panelMoveRequested("left")
            }

            ActionButton {
                id: moveToRight

                width: parent.width / 3
                height: width
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                hoverScalesIcons: false

                actionIconURL: "image://ThemeIcons/arrow-right"

                onTriggered: quarkProxy.panelMoveRequested("right")
            }
        }

        ActionButton {
            id: addQuarkButton

            visible: enableSettingsModeButton.settingsMode
            width: isVert ? quarkDisplayRoot.width : quarkDisplayRoot.height
            height: width

            actionIconURL: "image://ThemeIcons/list-add"
            textTooltip: addQuarkTooltip

            onTriggered: commonJS.showTooltip(addQuarkButton, function(x, y) { quarkProxy.quarkAddRequested(x, y) })
        }

        ActionButton {
            id: setQuarkOrderButton
            visible: enableSettingsModeButton.settingsMode
            width: isVert ? quarkDisplayRoot.width : quarkDisplayRoot.height
            height: width

            actionIconURL: "image://ThemeIcons/format-list-unordered"
            textTooltip: quarkOrderTooltip

            onTriggered: commonJS.showTooltip(setQuarkOrderButton, function(x, y) { quarkProxy.quarkOrderRequested(x, y) })
        }

        ActionButton {
            id: showPanelSettings
            visible: enableSettingsModeButton.settingsMode
            width: isVert ? quarkDisplayRoot.width : quarkDisplayRoot.height
            height: width

            actionIconURL: "image://ThemeIcons/configure-toolbars"
            textTooltip: showPanelSettingsTooltip

            onTriggered: quarkProxy.panelSettingsRequested()
        }

        ActionButton {
            id: enableSettingsModeButton
            width: isVert ? quarkDisplayRoot.width : quarkDisplayRoot.height
            height: width

            actionIconURL: "image://ThemeIcons/configure"
            textTooltip: settingsModeTooltip

            property bool settingsMode: false
            onTriggered: { isHighlight = !isHighlight; settingsMode = !settingsMode; }
        }
    }

    GridLayout {
        id: itemsView

        anchors.top: parent.top
        anchors.left: parent.left
        height: isVert ? parent.height - ownStuffGrid.height : parent.height
        width: isVert ? parent.width : parent.width - ownStuffGrid.width

        flow: isVert ? GridLayout.TopToBottom : GridLayout.LeftToRight

        rowSpacing: quarkSpacing
        columnSpacing: quarkSpacing

        clip: true

        Repeater {
            id: itemsRepeater
            model: itemsModel

            Item {
                id: itemsDelegate

                property bool isExpandable: itemLoader.item.isExpandable == undefined ? false : itemLoader.item.isExpandable
                implicitWidth: itemLoader.item.implicitWidth
                implicitHeight: itemLoader.item.implicitHeight

                Layout.fillWidth: isExpandable
                Layout.fillHeight: isExpandable

                Loader {
                    id: itemLoader

                    property real quarkBaseSize: isVert ? itemsView.width : itemsView.height

                    source: sourceURL
                    active: false

                    width: parent.width
                    height: parent.height

                    clip: true

                    Component.onCompleted: quarkProxy.instantiateQuark(sourceURL, this)
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
                    radius: Math.min(width, height) / 10

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
            }
        }
    }
}
