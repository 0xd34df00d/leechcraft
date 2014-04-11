import QtQuick 1.1
import Effects 1.0

Rectangle {
    id: rootRect

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

    ListView {
        id: artistsView

        anchors.fill: parent

        model: artistsModel

        delegate: Rectangle {
            radius: 5
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

            border.width: 1
            border.color: colorProxy.color_TextBox_BorderColor
            smooth: true

            height: 2 * artistNameLabel.paintedHeight
            width: artistsView.width

            Text {
                id: artistNameLabel
                text: artistName
                font.bold: true
                font.underline: true
                font.pointSize: 12
                color: colorProxy.color_TextBox_TitleTextColor
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 5
            }
        }
    }
}

