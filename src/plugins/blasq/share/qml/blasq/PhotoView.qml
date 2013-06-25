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

    function showImage(url) {
        if (url.length == 0) {
            fullSizeImage.state = "hidden"
            fullSizeImage.source = undefined
        } else {
            fullSizeImage.state = "loading"
            fullSizeImage.source = url
        }
    }

    Image {
        id: fullSizeImage

        z: collectionThumbsView.z + 1

        anchors.fill: parent
        anchors.margins: 32

        state: "hidden"

        states: [
            State {
                name: "hidden"
                PropertyChanges { target: fullSizeImage; opacity: 0; scale: 0 }
            },
            State {
                name: "loading"
                PropertyChanges {  }
            },
            State {
                name: "visible"
                PropertyChanges { target: fullSizeImage; opacity: 1; scale: 1 }
            }
        ]

        transitions: Transition {
            PropertyAnimation { properties: "opacity"; duration: 300; easing.type: Easing.OutSine }
        }

        MouseArea {
            anchors.fill: parent
            onReleased: fullSizeImage.state = "hidden"
        }

        onStatusChanged: if (status == Image.Ready) state = "visible"
    }

    GridView {
        id: collectionThumbsView

        visible: listingMode

        anchors.fill: parent
        cellWidth: 200
        cellHeight: 200

        property real horzMargin: cellWidth / 20
        property real vertMargin: cellHeight / 20

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

                    smooth: true

                    Image {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: nameLabel.top
                        anchors.margins: 2

                        source: smallThumb

                        smooth: true
                        fillMode: Image.PreserveAspectFit
                    }

                    Text {
                        id: nameLabel

                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.margins: 2

                        text: name

                        elide: Text.ElideMiddle
                        horizontalAlignment: Text.AlignHCenter
                        color: colorProxy.color_TextBox_TitleTextColor
                    }
                }
            }
        }
    }
}
