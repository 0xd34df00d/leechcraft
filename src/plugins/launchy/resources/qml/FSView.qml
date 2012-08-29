import QtQuick 1.0

Rectangle {
    id: rootRect
    anchors.fill: parent
    focus: true
    color: "#99000000"

    signal closeRequested()

    Keys.onEscapePressed: rootRect.closeRequested()

    Rectangle {
        id: catsContainer

        color: "#99222222"
        width: 200

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        ListView {
            id: catsView
            anchors.fill: parent
            spacing: 10

            model: VisualDataModel {
                model: itemsModel

                delegate: Rectangle {
                    id: catsViewDelegate

                    width: catsView.width
                    height: 20

                    radius: 5
                    color: categoryMouseArea.containsMouse ? "#ff000000" : "#00000000"

                    Behavior on color { PropertyAnimation {} }

                    Text {
                        text: categoryName
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 5
                        font.italic: true
                        font.pointSize: 12
                        color: "#dddddd"
                    }

                    MouseArea {
                        id: categoryMouseArea
                        anchors.fill: parent
                        hoverEnabled: true

                        onClicked: {
                            itemsView.model.rootIndex = catsView.model.modelIndex(index)
                        }
                    }
                }
            }
        }
    }

    GridView {
        id: itemsView
        anchors.left: catsContainer.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        model: VisualDataModel {
            model: itemsModel

            delegate: Rectangle {
                width: 32
                height: 32

                Text { text: "text"; anchors.fill: parent }
            }
        }
    }
}
