import QtQuick 2.3
import QtQuick.Window 2.1
import org.LC.common 1.0

Window {
    id: rootWindow

    width: loadView.cellWidth * 2
    height: loadView.cellHeight * Math.ceil(loadView.count / 2)

    flags: Qt.ToolTip

    property variant loadModel
    property variant cpuProxy
    property bool showIOTime
    property bool showLowTime
    property bool showMediumTime
    property bool showHighTime

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

    Rectangle {
        id: rootRect

        anchors.fill: parent

        smooth: true
        radius: 5

        function beforeDelete() {}

        property alias isHovered: wholeArea.containsMouse

        MouseArea {
            anchors.fill: parent
            id: wholeArea
            hoverEnabled: true
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
                                    enableIf(loadObj.lowHist, showLowTime),
                                    enableIf(loadObj.mediumHist, showMediumTime),
                                    enableIf(loadObj.highHist, showHighTime)
                                ) },
                            { color: "blue", points: zipN(
                                    enableIf(loadObj.ioHist, showIOTime),
                                    enableIf(loadObj.lowHist, showLowTime),
                                    enableIf(loadObj.mediumHist, showMediumTime)
                                ) },
                            { color: "yellow", points: zipN(
                                    enableIf(loadObj.ioHist, showIOTime),
                                    enableIf(loadObj.lowHist, showLowTime)
                                ) },
                            { color: "green", points: enableIf(loadObj.ioHist, showIOTime) }
                        ]

                    leftAxisEnabled: true
                    leftAxisTitle: qsTr("Load, %")
                    yGridEnabled: true

                    plotTitle: "CPU " + cpuIdx + ": " + momentalLoadStr

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
}
