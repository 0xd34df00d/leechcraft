import QtQuick 1.1
import org.LC.common 1.0

Rectangle {
    width: 500
    height: Math.min(400, rulesListView.contentHeight)

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

    ListView {
        id: rulesListView
        anchors.fill: parent

        model: rulesManager.getRulesModel()

        delegate: Rectangle {
            width: rulesListView.width
            height: ruleEnableButton.height + 8

            smooth: true

            radius: 3
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: colorProxy.color_TextBox_TopColor
                }
                GradientStop {
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

                actionIconURL: "image://ThemeIcons/dialog-ok"

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
        }
    }
}
