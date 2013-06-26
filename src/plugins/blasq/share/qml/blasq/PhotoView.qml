import QtQuick 1.0
import Effects 1.0
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
        fullSizeImage.source = url
    }

    Image {
        id: fullSizeImage

        z: collectionThumbsView.z + 1

        anchors.centerIn: parent
        width: Math.min(sourceSize.width, parent.width - 32)
        height: Math.min(sourceSize.height, parent.height - 32)

        fillMode: Image.PreserveAspectFit

        state: "hidden"
        states: [
            State {
                name: "hidden"
                PropertyChanges { target: fullSizeImage; opacity: 0 }
                PropertyChanges { target: photoViewBlur; blurRadius: 0 }
            },
            State {
                name: "loading"
                PropertyChanges { target: photoViewBlur; blurRadius: 3 }
            },
            State {
                name: "displayed"
                PropertyChanges { target: fullSizeImage; opacity: 1 }
                PropertyChanges { target: photoViewBlur; blurRadius: 10 }
            }
        ]

        transitions: Transition {
            PropertyAnimation { properties: "opacity"; duration: 300; easing.type: Easing.OutSine }
            PropertyAnimation { target: viewBlur; property: "blurRadius"; duration: 300; easing.type: Easing.OutSine }
        }

        MouseArea {
            anchors.fill: parent
            onReleased: rootRect.showImage("")
        }

        onStatusChanged: {
            switch (status) {
            case Image.Ready:
                state = "displayed"
                break;
            case Image.Loading:
                state = "loading"
                break;
            case Image.Null:
                state = "hidden"
                break;
            case Image.Error:
                state = "hidden"
                break;
            }
        }
    }

    GridView {
        id: collectionThumbsView

        visible: listingMode

        anchors.fill: parent
        cellWidth: 200
        cellHeight: 200

        property real horzMargin: cellWidth / 20
        property real vertMargin: cellHeight / 20

        effect: Blur {
            id: photoViewBlur
            blurRadius: 0.0
        }

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

                    MouseArea {
                        anchors.fill: parent
                        onReleased: rootRect.showImage(original)
                    }
                }
            }
        }
    }
}
