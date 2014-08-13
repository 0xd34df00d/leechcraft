import QtQuick 2.2

Rectangle {
    width: 500
    height: Math.min(400, mailListView.contentHeight)

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

    ListView {
        id: mailListView
        anchors.fill: parent

        model: mailListModel

        delegate: Rectangle {
            width: mailListView.width
            height: subjectLabel.height + dateLabel.height + authorLabel.height + summaryLabel.height

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

            Text {
                id: subjectLabel
                anchors.top: parent.top
                anchors.topMargin: 2
                anchors.left: parent.left
                anchors.leftMargin: 2
                anchors.right: parent.right
                anchors.rightMargin: 2

                text: subject

                color: colorProxy.color_TextBox_TitleTextColor
                font.bold: true
                elide: Text.ElideRight
            }

            Text {
                id: dateLabel
                anchors.top: subjectLabel.bottom
                anchors.topMargin: 2
                anchors.right: parent.right
                anchors.rightMargin: 2

                text: modifiedDate

                color: colorProxy.color_TextBox_TextColor
            }

            Text {
                id: authorLabel
                anchors.top: subjectLabel.bottom
                anchors.topMargin: 2
                anchors.left: parent.left
                anchors.leftMargin: 2
                anchors.right: dateLabel.left
                anchors.rightMargin: 2

                text: authorName + " <" + authorEmail + ">"

                color: colorProxy.color_TextBox_TextColor
                font.italic: true
                elide: Text.ElideRight
            }

            Text {
                id: summaryLabel
                anchors.top: authorLabel.bottom
                anchors.topMargin: 2
                anchors.left: parent.left
                anchors.leftMargin: 2
                anchors.right: parent.right
                anchors.rightMargin: 2

                text: summary

                color: colorProxy.color_TextBox_Aux1TextColor
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
        }
    }
}
