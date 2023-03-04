import QtQuick 2.3
import QtQuick.Controls 2.15

Rectangle {
    id: rootRect

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

    GridView {
        id: thumbsView

        focus: true

        anchors.fill: parent
        interactive: false

        model: thumbsModel

        highlightFollowsCurrentItem: false

        currentIndex: initialIndex

        displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.InOutQuad }
        }

        function calculateDims(count) {
            let rows = Math.floor(Math.sqrt(count));
            let cols = rows;
            if (rows * cols < count)
                ++cols;
            if (rows * cols < count)
                ++rows;
            return { rows, cols };
        }

        readonly property var dims: calculateDims(initialCount)

        cellWidth: width / dims.cols
        cellHeight: height / dims.rows

        readonly property int horizontalMargins: cellWidth / 10
        readonly property int verticalMargins: cellHeight / 10

        function selectCurrent() { view.selectItem(currentIndex); }
        function deleteCurrent() { currentIndex = view.deleteItem(currentIndex); }

        Keys.onEscapePressed: view.finish()
        Keys.onEnterPressed: selectCurrent()
        Keys.onReturnPressed: selectCurrent()
        Keys.onSpacePressed: selectCurrent()
        Keys.onDeletePressed: deleteCurrent()

        function navigate(delta, wrapAround) {
            if (currentItem) { currentIndex += delta; }
            if (currentIndex < 0 || currentIndex >= count) { currentIndex = wrapAround; }
        }

        function next() { navigate(+1, 0); }
        function prev() { navigate(-1, count - 1); }

        Keys.onLeftPressed: prev()
        Keys.onRightPressed: next()
        Keys.onDownPressed: {
            if (dims.rows === 1) {
                next();
            } else {
                const wrapAround = currentItem ? currentIndex % dims.cols : 0;
                navigate(+dims.cols, wrapAround);
            }
        }
        Keys.onUpPressed: {
            if (dims.rows === 1) {
                prev();
            } else {
                let wrapAround = currentItem ? currentIndex + (dims.rows - 1) * dims.cols : 0;
                if (wrapAround >= count) {
                    wrapAround -= dims.cols;
                }
                navigate(-dims.cols, wrapAround);
            }
        }

        delegate: Rectangle {
            id: thumbsItem

            width: thumbsView.cellWidth
            height: thumbsView.cellHeight

            color: "transparent"

            GridView.onRemove: SequentialAnimation {
                PropertyAction { target: thumbsItem; property: "GridView.delayRemove"; value: true }
                NumberAnimation { target: thumbsItem; property: "opacity"; to: 0; duration: 100; easing.type: Easing.InOutQuad }
                PropertyAction { target: thumbsItem; property: "GridView.delayRemove"; value: false }
            }

            Image {
                id: thumbsImage

                source: "image://thumbs/" + thumbId
                smooth: true
                cache: false
                fillMode: Image.PreserveAspectFit

                anchors.centerIn: parent
                anchors.fill: parent
                anchors.leftMargin: thumbsView.horizontalMargins
                anchors.rightMargin: thumbsView.horizontalMargins
                anchors.topMargin: thumbsView.verticalMargins
                anchors.bottomMargin: thumbsView.verticalMargins

                states: [
                    State {
                        name: "current"
                        when: thumbsView.currentIndex === index
                        PropertyChanges {
                            target: thumbsImage
                            anchors.leftMargin: 0
                            anchors.rightMargin: 0
                            anchors.topMargin: 0
                            anchors.bottomMargin: 0
                        }
                    }
                ]

                transitions: Transition {
                    NumberAnimation { properties: "anchors.leftMargin,anchors.rightMargin,anchors.topMargin,anchors.bottomMargin"; duration: 100 }
                }

                ToolButton {
                    x: parent.x + parent.width / 2 - parent.anchors.leftMargin + parent.paintedWidth / 2 - width
                    y: parent.y + parent.height / 2 - parent.anchors.topMargin - parent.paintedHeight / 2
                    width: 64
                    height: 64

                    icon.name: "window-close"
                    icon.width: width
                    icon.height: height
                    icon.color: "transparent"
                    display: AbstractButton.IconOnly

                    onClicked: {
                        if (index === thumbsView.currentIndex) {
                            thumbsView.deleteCurrent();
                        } else {
                            view.deleteItem(index);
                        }
                    }
                }
            }
        }
    }
}
