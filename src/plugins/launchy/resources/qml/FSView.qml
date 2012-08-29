import QtQuick 1.0

Rectangle {
    id: rootRect
    anchors.fill: parent
    focus: true
    color: "#e0000000"

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

                    color: categoryMouseArea.containsMouse ? "#aa000000" : "#00000000"
                    Behavior on color { PropertyAnimation {} }

                    Text {
                        text: categoryName

                        font.italic: true
                        font.pointSize: 12
                        color: "#cccccc"

                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 5
                    }

                    MouseArea {
                        id: categoryMouseArea
                        anchors.fill: parent
                        hoverEnabled: true

                        onClicked: {
                            itemsView.visible = true;
                            itemsView.model.rootIndex = catsView.model.modelIndex(index);
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
        visible: false

        cellWidth: 128
        cellHeight: 128

        model: VisualDataModel {
            model: itemsModel

            delegate: Rectangle {
                width: itemsView.cellWidth
                height: itemsView.cellHeight
                radius: 5

                color: itemMouseArea.containsMouse ? "#44FF6600" : "#00000000"
                Behavior on color { PropertyAnimation {} }

                Image {
                    width: 96
                    height: 96
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top

                    source: "image://appicon/" + itemIcon
                    smooth: true
                }

                Text {
                    text: itemName

                    width: parent.width
                    font.pointSize: 10
                    color: "#eeeeee"

                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 3

                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    horizontalAlignment: Text.AlignHCenter
                }

                MouseArea {
                    id: itemMouseArea
                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: {
                    }
                }
            }
        }
    }
}
