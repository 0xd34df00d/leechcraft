import QtQuick 1.1
import org.LC.common 1.0

Rectangle {
    id: rootRect
    width: loadView.cellWidth * 2
    height: loadView.cellHeight * Math.ceil(loadView.count / 2)

    smooth: true
    radius: 5

    opacity: 0.001

    signal closeRequested()

    function beforeDelete() {}

    property alias isHovered: wholeArea.containsMouse

    MouseArea {
        anchors.fill: parent
        id: wholeArea
        hoverEnabled: true
    }

    function zipN(first) {
        for (var i = 1; i < arguments.length; ++i) {
            var other = arguments[i];
            first = cpuProxy.sumPoints(first, other);
        }
        return first;
    }

    function enableIf(array, flag) {
        return cpuProxy.enableIf(array, flag);
    }

    Timer {
        id: showTimer
        repeat: false
        interval: 0
        onTriggered: { rootRect.opacity = 1; }
    }

    GridView {
        id: loadView

        model: loadModel

        anchors.fill: parent

        cellWidth: maxPlotX + xExtent
        cellHeight: 130

        property int desiredRows: Math.ceil(Math.sqrt(count))

        property int maxPlotX: 200
        property int xExtent: 0

        function setPlotParams(xex, maxX) {
            if (xExtent !== xex)
                xExtent = xex;
            if (maxPlotX !== maxX)
                maxPlotX = maxX;

            if (rootRect.opacity !== 1)
                showTimer.start();
        }

        delegate: Rectangle {
            width: loadView.cellWidth
            height: loadView.cellHeight

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

            Plot {
                id: plot

                anchors.top: parent.top

                width: loadView.cellWidth
                height: loadView.cellHeight

                multipoints: [
                        { color: "red", points: zipN(
                                enableIf(loadObj.ioHist, showIOTime),
                                loadObj.lowHist,
                                loadObj.mediumHist,
                                loadObj.highHist
                            ) },
                        { color: "blue", points: zipN(
                                enableIf(loadObj.ioHist, showIOTime),
                                loadObj.lowHist,
                                loadObj.mediumHist
                            ) },
                        { color: "yellow", points: zipN(
                                enableIf(loadObj.ioHist, showIOTime),
                                loadObj.lowHist
                            ) },
                        { color: "green", points: enableIf(loadObj.ioHist, showIOTime) }
                    ]

                leftAxisEnabled: true
                leftAxisTitle: qsTr("Load, %")
                yGridEnabled: true

                plotTitle: "CPU " + cpuIdx + "; " + momentalLoadStr

                minYValue: 0
                maxYValue: 100
                minXValue: 0
                maxXValue: loadObj.getMaxX()

                alpha: 1
                background: "transparent"
                textColor: colorProxy.color_TextBox_TextColor
                gridLinesColor: colorProxy.color_TextBox_Aux2TextColor

                onExtentsChanged: loadView.setPlotParams(xExtent, loadObj.getMaxX())
            }
        }
    }
}
