import QtQuick 2.3
import org.LC.common 1.0

Rectangle {
    width: 500
    height: Math.min(400, rulesListView.contentHeight + actionsView.contentHeight)

    radius: 3
    gradient: Gradient {
        GradientStop {
            position: 0
            color: colorProxy.color_TextView_TopColor
        }
        GradientStop {
            position: 1
            color: colorProxy.color_TextView_BottomColor
        }
    }

    smooth: true

    signal closeRequested()

    GridView {
        id: actionsView

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: contentHeight

        cellWidth: 36
        cellHeight: 36

        model: proxy.getActionsModel()

        delegate: Rectangle {
            width: actionsView.cellWidth
            height: actionsView.cellHeight

            color: "transparent"

            ActionButton {
                width: 32
                height: 32
                anchors.centerIn: parent

                actionIconURL: "image://ThemeIcons/" + iconName
                isHighlight: isActionChecked
                onTriggered: proxy.getActionsModel().triggerAction(index)
            }
        }
    }

    ListView {
        id: rulesListView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: actionsView.bottom
        anchors.bottom: parent.bottom

        model: rulesManager.getRulesModel()
        clip: true

        delegate: Rectangle {
            id: itemRect

            width: rulesListView.width
            height: ruleEnableButton.height + 8

            smooth: true

            radius: 3
            gradient: Gradient {
                GradientStop {
                    id: upperStop
                    position: 0
                    color: colorProxy.color_TextBox_TopColor
                }
                GradientStop {
                    id: lowerStop
                    position: 1
                    color: colorProxy.color_TextBox_BottomColor
                }
            }

            border.color: colorProxy.color_TextBox_BorderColor
            border.width: 1

            ActionButton {
                id: ruleEnableButton

                width: 24
                height: width
                anchors.left: parent.left
                anchors.leftMargin: 4
                anchors.verticalCenter: parent.verticalCenter

                actionIconURL: isRuleEnabled ? "image://ThemeIcons/dialog-ok" : "image://ThemeIcons/dialog-cancel"

                isHighlight: isRuleEnabled
                onTriggered: rulesManager.setRuleEnabled(index, !isRuleEnabled)
            }

            Text {
                id: ruleNameLabel

                anchors.verticalCenter: parent.verticalCenter
                anchors.left: ruleEnableButton.right
                anchors.leftMargin: 4
                anchors.right: parent.right
                anchors.rightMargin: 4

                text: ruleName
                color: colorProxy.color_TextBox_TitleTextColor
                elide: Text.ElideMiddle
            }

            MouseArea {
                id: rectMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onReleased: ruleEnableButton.triggered()
            }

            states: [
                State {
                    name: "hovered"
                    when: rectMouseArea.containsMouse
                    PropertyChanges { target: itemRect; border.color: colorProxy.color_TextBox_HighlightBorderColor}
                    PropertyChanges { target: upperStop; color: colorProxy.color_TextBox_HighlightTopColor }
                    PropertyChanges { target: lowerStop; color: colorProxy.color_TextBox_HighlightBottomColor }
                }
            ]

            transitions: [
                Transition {
                    from: ""
                    to: "hovered"
                    reversible: true
                    PropertyAnimation { properties: "border.color"; duration: 200 }
                    PropertyAnimation { properties: "color"; duration: 200 }
                }
            ]
        }
    }
}
