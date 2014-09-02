import QtQuick 2.3
import org.LC.common 1.0

Rectangle {
    id: rootRect
    anchors.fill: parent
    focus: true
    gradient: Gradient {
        GradientStop {
            position: 0
            color: colorProxy.setAlpha(colorProxy.color_TextView_TopColor, 0.8)
        }
        GradientStop {
            position: 1
            color: colorProxy.setAlpha(colorProxy.color_TextView_BottomColor, 0.8)
        }
    }

    signal closeRequested()
    signal itemSelected(string id)
    signal itemBookmarkRequested(string id)
    signal categorySelected(int index)

    Keys.onEscapePressed: rootRect.closeRequested()

    Keys.forwardTo: [itemsView, appsFilterInput]

    Keys.onLeftPressed: itemsView.moveCurrentIndexLeft()
    Keys.onRightPressed: itemsView.moveCurrentIndexRight()
    Keys.onBacktabPressed: itemsView.moveCurrentIndexLeft()
    Keys.onTabPressed: itemsView.moveCurrentIndexRight()
    Keys.onUpPressed: itemsView.moveCurrentIndexUp()
    Keys.onDownPressed: itemsView.moveCurrentIndexDown()
    Keys.onEnterPressed: trigger()
    Keys.onReturnPressed: trigger()

    function trigger() {
        if (itemsView.currentItem == null)
            itemsView.moveCurrentIndexRight();
        itemsView.currentItem.trigger();
    }

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
            anchors.topMargin: 5
            currentIndex: 0

            model: catsModel

            onCurrentIndexChanged: {
                    rootRect.categorySelected(currentIndex);
                    itemsView.currentIndex = -1;
                    appsFilterInput.text = "";
                }

            delegate: Rectangle {
                id: catsViewDelegate

                width: catsView.width
                height: 30
                radius: 5

                gradient: Gradient {
                    GradientStop {
                        position: 0
                        color: categoryMouseArea.containsMouse ?
                                colorProxy.color_ToolButton_HoveredTopColor :
                                    (catsView.currentIndex == index && !appsFilterInput.text.length ?
                                        colorProxy.color_ToolButton_SelectedTopColor :
                                        colorProxy.color_ToolButton_TopColor)
                        Behavior on color { PropertyAnimation {} }
                    }
                    GradientStop {
                        position: 1
                        color: categoryMouseArea.containsMouse ?
                                colorProxy.color_ToolButton_HoveredBottomColor :
                                    (catsView.currentIndex == index && !appsFilterInput.text.length ?
                                        colorProxy.color_ToolButton_SelectedBottomColor :
                                        colorProxy.color_ToolButton_BottomColor)
                        Behavior on color { PropertyAnimation {} }
                    }
                }

                Image {
                    id: categoryIconImage
                    width: 24
                    height: 24
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 5

                    source: "image://appicon/" + categoryIcon
                    smooth: true
                }

                Text {
                    text: categoryName

                    font.italic: true
                    font.pointSize: 12
                    color: colorProxy.color_ToolButton_TextColor

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: categoryIconImage.right
                    anchors.leftMargin: 5
                }

                MouseArea {
                    id: categoryMouseArea
                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: catsView.currentIndex = index
                }
            }

            section.criteria: ViewSection.FullString
            section.property: "categoryType"
            section.delegate: Rectangle {
                width: catsView.width
                height: 2
                radius: 1

                color: colorProxy.color_ToolButton_BorderColor
                smooth: true
            }
        }
    }

    Rectangle {
        id: appsFilterInputContainer

        anchors.left: catsContainer.right
        anchors.leftMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.right: parent.right
        height: 22

        color: colorProxy.color_Panel_TopColor

        TextInput {
            id: appsFilterInput
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 4
            font.pointSize: 12
            font.italic: true
            color:  colorProxy.color_Panel_TextColor
            focus: true
        }

        Binding {
            target: launchyProxy
            property: "appFilterText"
            value: appsFilterInput.text
        }
    }

    GridView {
        id: itemsView
        anchors.left: catsContainer.right
        anchors.top: appsFilterInputContainer.bottom
        anchors.bottom: itemDescriptionLabel.top
        anchors.right: parent.right
        anchors.margins: 5
        currentIndex: -1

        cellWidth: 160
        cellHeight: 128

        model: itemsModel

        focus: true
        keyNavigationWraps: true

        delegate: Rectangle {
            id: itemsViewDelegate

            width: itemsView.cellWidth
            height: itemsView.cellHeight
            radius: 5

            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: itemMouseArea.containsMouse ?
                            colorProxy.color_ToolButton_HoveredTopColor :
                                (itemsView.currentIndex == index ?
                                    colorProxy.color_ToolButton_SelectedTopColor :
                                    colorProxy.setAlpha(colorProxy.color_ToolButton_TopColor, 0.5))
                    Behavior on color { PropertyAnimation {} }
                }
                GradientStop {
                    position: 1
                    color: itemMouseArea.containsMouse ?
                            colorProxy.color_ToolButton_HoveredBottomColor :
                                (itemsView.currentIndex == index ?
                                    colorProxy.color_ToolButton_SelectedBottomColor :
                                    colorProxy.setAlpha(colorProxy.color_ToolButton_BottomColor, 0.5))
                    Behavior on color { PropertyAnimation {} }
                }
            }

            Image {
                width: 64
                height: 64
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 16

                source: "image://appicon/" + itemIcon
                smooth: true
            }

            Text {
                text: itemName

                width: parent.width
                font.pointSize: 10
                color: colorProxy.color_ToolButton_TextColor

                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 16

                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                horizontalAlignment: Text.AlignHCenter
            }

            MouseArea {
                id: itemMouseArea
                anchors.fill: parent
                hoverEnabled: true

                onClicked: trigger()

                onEntered: itemDescriptionLabel.text = itemDescription
                onExited: itemDescriptionLabel.text = ""
            }

            ActionButton {
                id: addToFavs

                anchors.right: parent.right
                anchors.bottom: parent.bottom
                width: 24
                height: 24

                actionIconURL: "image://theme/favorites"

                onTriggered: rootRect.itemBookmarkRequested(itemID)
                visible: !isItemFavorite
            }

            GridView.onAdd: SequentialAnimation {
                NumberAnimation { target: itemsViewDelegate; property: "opacity"; from: 0; to: 1; duration: 250; easing.type: Easing.InOutQuad }
            }

            function trigger() {
                rootRect.itemSelected(itemID);
            }
        }
    }

    Text {
        id: itemDescriptionLabel

        height: 16

        color: colorProxy.color_TextView_Aux1TextColor

        anchors.left: catsContainer.right
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        clip: true
    }
}
