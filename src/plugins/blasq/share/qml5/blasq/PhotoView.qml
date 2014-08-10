import QtQuick 2.3
import QtGraphicalEffects 1.0
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
        rootRect.singleImageMode(url.toString().length > 0)
    }

    signal singleImageMode(bool mode)
    signal imageSelected(string id)
    signal toggleSelectionSet(string id)
    signal imageOpenRequested(variant url)
    signal imageDownloadRequested(variant url)
    signal copyURLRequested(variant url)
    signal deleteRequested(string id)
    signal albumSelected(variant index)

    property string currentImageId
    property real cellSize: 200
    property real imageZoom: 100

    Flickable {
        z: collectionThumbsView.z + 1
        anchors.centerIn: parent
        width: Math.min(contentWidth, parent.width)
        height: Math.min(contentHeight, parent.height)

        contentWidth: fullSizeImage.width * fullSizeImage.scale
        contentHeight: fullSizeImage.height * fullSizeImage.scale
        contentX: Math.max((contentWidth - width) / 2, 0)
        contentY: Math.max((contentHeight - height) / 2, 0)
        opacity: fullSizeImage.opacity

        Image {
            id: fullSizeImage

            fillMode: Image.PreserveAspectFit
            width: sourceSize.width * rootRect.imageZoom / 100
            height: sourceSize.height * rootRect.imageZoom / 100

            smooth: true

            Behavior on width { PropertyAnimation { duration: 150; easing.type: Easing.InOutSine } }
            Behavior on height { PropertyAnimation { duration: 150; easing.type: Easing.InOutSine } }

            state: "hidden"
            states: [
                State {
                    name: "hidden"
                    PropertyChanges { target: fullSizeImage; opacity: 0 }
                    PropertyChanges { target: photoViewBlur; radius: 0 }
                    PropertyChanges { target: loadProgress; opacity: 0 }
                },
                State {
                    name: "loading"
                    PropertyChanges { target: photoViewBlur; radius: 3 }
                    PropertyChanges { target: loadProgress; opacity: 1 }
                },
                State {
                    name: "displayed"
                    PropertyChanges { target: fullSizeImage; opacity: 1 }
                    PropertyChanges { target: photoViewBlur; radius: 10 }
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
    }

    VisualDataModel {
        id: collectionVisualModel
        model: collectionModel

        rootIndex: collRootIndex
        onRootIndexChanged: {
            collectionThumbsView.model = undefined
            collectionThumbsView.model = collectionVisualModel
        }

        delegate: ActionButton {
            id: delegateRoot

            width: collectionThumbsView.cellWidth
            height: collectionThumbsView.cellHeight

            onTriggered: {
                if (itemType != Blasq.ImageItem) {
                    rootRect.albumSelected(collectionVisualModel.modelIndex(index))
                } else {
                    rootRect.showImage(original)
                    rootRect.imageSelected(imageId)
                    rootRect.currentImageId = imageId
                }
            }

            Rectangle {
                id: itemRect

                anchors.fill: parent
                anchors.leftMargin: collectionThumbsView.horzMargin
                anchors.rightMargin: collectionThumbsView.horzMargin
                anchors.topMargin: collectionThumbsView.vertMargin
                anchors.bottomMargin: collectionThumbsView.vertMargin

                property bool isCurrent: imageId == rootRect.currentImageId || isSelected

                radius: 5
                gradient: Gradient {
                    GradientStop {
                        position: 0
                        color: itemRect.isCurrent ?
                                colorProxy.color_TextBox_HighlightTopColor :
                                colorProxy.color_TextBox_TopColor
                        Behavior on color { PropertyAnimation {} }
                    }
                    GradientStop {
                        position: 1
                        color: itemRect.isCurrent ?
                                colorProxy.color_TextBox_HighlightBottomColor :
                                colorProxy.color_TextBox_BottomColor
                        Behavior on color { PropertyAnimation {} }
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
                        source: width > smallThumbSize.width ? mediumThumb : smallThumb
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

                property bool isHovered: delegateRoot.isHovered ||
                            openInBrowserAction.isHovered ||
                            downloadOriginalAction.isHovered ||
                            copyURLAction.isHovered ||
                            deleteAction.isHovered ||
                            selectionAction.isHovered

                Column {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    visible: itemType == Blasq.ImageItem

                    ActionButton {
                        id: selectionAction

                        width: 24
                        height: width

                        visible: imageSelectionMode
                        opacity: itemRect.isHovered ? 1 : 0

                        Behavior on opacity { PropertyAnimation {} }

                        actionIconURL: isSelected ? "image://ThemeIcons/list-remove" : "image://ThemeIcons/list-add"
                        textTooltip: isSelected ? qsTr("Add to the selection") : qsTr ("Remove from the selection")
                        onTriggered: rootRect.toggleSelectionSet(imageId)
                    }

                    ActionButton {
                        id: openInBrowserAction

                        width: 24
                        height: width

                        visible: !imageSelectionMode
                        opacity: itemRect.isHovered ? 1 : 0
                        Behavior on opacity { PropertyAnimation {} }

                        actionIconURL: "image://ThemeIcons/go-jump-locationbar"
                        textTooltip: qsTr("Open in browser")
                        onTriggered: rootRect.imageOpenRequested(original)
                    }

                    ActionButton {
                        id: downloadOriginalAction

                        width: 24
                        height: width

                        visible: !imageSelectionMode
                        opacity: itemRect.isHovered ? 1 : 0
                        Behavior on opacity { PropertyAnimation {} }

                        actionIconURL: "image://ThemeIcons/download"
                        textTooltip: qsTr("Download the original image")
                        onTriggered: rootRect.imageDownloadRequested(original)
                    }

                    ActionButton {
                        id: copyURLAction

                        width: 24
                        height: width

                        visible: !imageSelectionMode
                        opacity: itemRect.isHovered ? 1 : 0
                        Behavior on opacity { PropertyAnimation {} }

                        actionIconURL: "image://ThemeIcons/edit-copy"
                        textTooltip: qsTr("Copy image URL")
                        onTriggered: rootRect.copyURLRequested(original)
                    }

                    ActionButton {
                        id: deleteAction

                        width: 24
                        height: width

                        visible: !imageSelectionMode
                        opacity: itemRect.isHovered && supportsDeletes ? 1 : 0
                        Behavior on opacity { PropertyAnimation {} }

                        actionIconURL: "image://ThemeIcons/list-remove"
                        textTooltip: qsTr("Delete the image")
                        onTriggered: rootRect.deleteRequested(imageId)
                    }
                }
            }
        }
    }

    GridView {
        id: collectionThumbsView

        visible: listingMode && photoViewBlur.radius == 0

        anchors.fill: parent
        cellWidth: rootRect.cellSize
        cellHeight: rootRect.cellSize

        property real horzMargin: cellWidth / 20
        property real vertMargin: cellHeight / 20

        model: collectionVisualModel
    }

    RecursiveBlur {
        id: photoViewBlur

        anchors.fill: collectionThumbsView
        source: collectionThumbsView
        radius: 0
        loops: 30
        transparentBorder: true

        visible: listingMode && radius != 0
    }
}
