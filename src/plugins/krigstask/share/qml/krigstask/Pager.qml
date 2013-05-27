import QtQuick 1.1

Rectangle {
    id: rootRect

    property int singleListWidth: 200
    width: desktopsView.count * singleListWidth
    height: 500

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

    ListView {
        id: desktopsView

        anchors.fill: parent

        model: desktopsModel
        orientation: ListView.Horizontal

        delegate: ListView {
            model: subModel

            width: rootRect.singleListWidth
            height: count * 100

            delegate: Rectangle {
                width: rootRect.singleListWidth
                height: 100

                border.width: 1
                border.color: colorProxy.color_TextBox_BorderColor

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

                Text {
                    id: appNameLabel

                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right

                    text: appName
                    elide: Text.ElideRight

                    color: colorProxy.color_TextBox_TitleTextColor
                    font.pointSize: 14
                }
            }
        }
    }
}
