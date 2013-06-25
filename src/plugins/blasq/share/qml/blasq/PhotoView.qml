import QtQuick 1.0
import org.LC.common 1.0

Rectangle {
    id: rootRect

    anchors.fill: parent

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

    GridView {
        id: collectionThumbsView

        visible: listingMode

        anchors.fill: parent
        cellWidth: 200
        cellHeight: 200

        property real horzMargin: cellWidth / 10
        property real vertMargin: cellHeight / 10

        model: VisualDataModel {
            model: collectionModel

            rootIndex: collRootIndex

            delegate: Item {
                width: collectionThumbsView.cellWidth
                height: collectionThumbsView.cellHeight

                Rectangle {
                    anchors.fill: parent
                    anchors.leftMargin: collectionThumbsView.horzMargin
                    anchors.rightMargin: collectionThumbsView.horzMargin
                    anchors.topMargin: collectionThumbsView.vertMargin
                    anchors.bottomMargin: collectionThumbsView.vertMargin

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

                    Image {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: nameLabel.top

                        source: smallThumb

                        smooth: true
                        fillMode: Image.PreserveAspectFit
                    }

                    Text {
                        id: nameLabel

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom

                        text: name
                        elide: Text.ElideMiddle
                    }
                }
            }
        }
    }
}
