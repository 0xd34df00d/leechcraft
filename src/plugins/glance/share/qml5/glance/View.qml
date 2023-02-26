import QtQuick 2.3

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

        function calculateDims(count) {
            let rows = Math.floor(Math.sqrt(count));
            let cols = rows;
            if (rows * cols < count)
                ++cols;
            if (rows * cols < count)
                ++rows;
            return { rows, cols };
        }

        readonly property var dims: calculateDims(count)

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
            width: thumbsView.cellWidth
            height: thumbsView.cellHeight

            color: "transparent"

            Image {
                source: index >= 0 ? "image://thumbs/" + index : ""

                smooth: true

                anchors.centerIn: parent
                anchors.fill: parent

                readonly property bool isCurrent: thumbsView.currentIndex === index

                anchors.leftMargin: isCurrent ? 0 : thumbsView.horizontalMargins
                anchors.rightMargin: isCurrent ? 0 : thumbsView.horizontalMargins
                anchors.topMargin: isCurrent ? 0 : thumbsView.verticalMargins
                anchors.bottomMargin: isCurrent ? 0 : thumbsView.verticalMargins

                fillMode: Image.PreserveAspectFit
            }
        }
    }
}
