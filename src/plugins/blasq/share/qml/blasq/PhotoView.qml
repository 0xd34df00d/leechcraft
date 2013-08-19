import QtQuick 1.0
import Effects 1.0
import org.LC.common 1.0
import org.LC.Blasq 1.0

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

    signal imageSelected(string id)
    signal imageOpenRequested(variant url)
    signal imageDownloadRequested(variant url)
    signal copyURLRequested(variant url)

    property string currentImageId

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
                PropertyChanges { target: loadProgress; opacity: 0 }
            },
            State {
                name: "loading"
                PropertyChanges { target: photoViewBlur; blurRadius: 3 }
                PropertyChanges { target: loadProgress; opacity: 1 }
            },
            State {
                name: "displayed"
                PropertyChanges { target: fullSizeImage; opacity: 1 }
                PropertyChanges { target: photoViewBlur; blurRadius: 10 }
                PropertyChanges { target: loadProgress; opacity: 0 }
            }
        ]

        transitions: Transition {
            PropertyAnimation { properties: "opacity"; duration: 300; easing.type: Easing.OutSine }
            PropertyAnimation { target: photoViewBlur; property: "blurRadius"; duration: 300; easing.type: Easing.OutSine }
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

        ProgressBar {
            id: loadProgress

            value: parent.progress * 100

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.bottom
            height: 12

            color: colorProxy.color_TextView_Aux3TextColor
        }
    }

    VisualDataModel {
        id: collectionVisualModel
        model: collectionModel

        rootIndex: collRootIndex
        onRootIndexChanged: {
            collectionThumbsView.model = undefined
            collectionThumbsView.model = collectionVisualModel
        }

        delegate: Item {
            width: collectionThumbsView.cellWidth
            height: collectionThumbsView.cellHeight

            Rectangle {
                id: itemRect

                anchors.fill: parent
                anchors.leftMargin: collectionThumbsView.horzMargin
                anchors.rightMargin: collectionThumbsView.horzMargin
                anchors.topMargin: collectionThumbsView.vertMargin
                anchors.bottomMargin: collectionThumbsView.vertMargin

                property bool isCurrent: imageId == rootRect.currentImageId

                radius: 5
                gradient: Gradient {
                    GradientStop {
                        position: 0
                        color: itemRect.isCurrent ?
                                colorProxy.color_TextBox_HighlightTopColor :
                                colorProxy.color_TextBox_TopColor
                    }
                    GradientStop {
                        position: 1
                        color: itemRect.isCurrent ?
                                colorProxy.color_TextBox_HighlightBottomColor :
                                colorProxy.color_TextBox_BottomColor
                    }
                }

                border.width: 1
                border.color: isCurrent ?
                        colorProxy.color_TextBox_HighlightBorderColor :
                        colorProxy.color_TextBox_BorderColor

                smooth: true

                Component {
                    id: photoImageComponent
                    Image {
                        source: smallThumb
                        smooth: true
                        fillMode: Image.PreserveAspectFit
                    }
                }

                Component {
                    id: collectionCollageComponent
                    Repeater {
                        id: collectionCollageRepeater
                        model: smallThumb
                        property real logScale: count ? Math.log(Math.E - 1 + count) : 1

                        Image {
                            source: modelData

                            width: imagesDisplay.width / collectionCollageRepeater.logScale
                            height: imagesDisplay.height / collectionCollageRepeater.logScale

                            property real norm: collectionCollageRepeater.count - 1
                            property real xA: imagesDisplay.width / 25
                            property real xB: (imagesDisplay.width - width) / 2 - xA * norm / 2

                            x: xA * index + xB
                            y: (imagesDisplay.height - height) / 2
                            transformOrigin: Item.Bottom
                            rotation: norm ? 15 * (index * 2 / norm - 1) : 0

                            smooth: true
                            fillMode: Image.PreserveAspectFit
                        }
                    }
                }

                Loader {
                    id: imagesDisplay
                    sourceComponent: itemType == Blasq.ImageItem ? photoImageComponent : collectionCollageComponent

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: nameLabel.top
                    anchors.margins: 2
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

                property bool isHovered: itemMouseArea.containsMouse ||
                            openInBrowserAction.isHovered ||
                            downloadOriginalAction.isHovered ||
                            copyURLAction.isHovered

                MouseArea {
                    id: itemMouseArea
                    anchors.fill: parent

                    hoverEnabled: true
                    onReleased: {
                        rootRect.showImage(original)
                        rootRect.imageSelected(imageId)

                        rootRect.currentImageId = imageId
                    }
                }

                ActionButton {
                    id: openInBrowserAction

                    anchors.top: parent.top
                    anchors.right: parent.right
                    width: 24
                    height: width

                    opacity: parent.isHovered ? 1 : 0
                    Behavior on opacity { PropertyAnimation {} }

                    actionIconURL: "image://ThemeIcons/go-jump-locationbar"
                    textTooltip: qsTr("Open in browser")
                    onTriggered: rootRect.imageOpenRequested(original)
                }

                ActionButton {
                    id: downloadOriginalAction

                    anchors.top: openInBrowserAction.bottom
                    anchors.right: parent.right
                    width: 24
                    height: width

                    opacity: parent.isHovered ? 1 : 0
                    Behavior on opacity { PropertyAnimation {} }

                    actionIconURL: "image://ThemeIcons/download"
                    textTooltip: qsTr("Download the original image")
                    onTriggered: rootRect.imageDownloadRequested(original)
                }

                ActionButton {
                    id: copyURLAction

                    anchors.top: downloadOriginalAction.bottom
                    anchors.right: parent.right
                    width: 24
                    height: width

                    opacity: parent.isHovered ? 1 : 0
                    Behavior on opacity { PropertyAnimation {} }

                    actionIconURL: "image://ThemeIcons/edit-copy"
                    textTooltip: qsTr("Copy image URL")
                    onTriggered: rootRect.copyURLRequested(original)
                }
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

        model: collectionVisualModel
    }
}
