import QtQuick 2.3
import QtQuick.Window 2.1
import org.LC.common 1.0

Window {
    width: loadView.cellWidth * 2
    height: loadView.cellHeight * Math.ceil(loadView.count / 2)

    flags: Qt.ToolTip

    property variant loadModel
    property variant cpuProxy

    function zipN(first) {
        for (var i = 1; i < arguments.length; ++i) {
            var other = arguments[i];
            first = cpuProxy.sumPoints(first, other);
        }
        return first;
    }

    Rectangle {
        id: rootRect

        anchors.fill: parent

        smooth: true
        radius: 5

        function beforeDelete() {}

        property alias isHovered: wholeArea.containsMouse

        MouseArea {
            id: wholeArea
            hoverEnabled: true
        }

        GridView {
            id: loadView

            model: loadModel

            anchors.fill: parent

            cellWidth: 400
            cellHeight: 100

            property int desiredRows: Math.ceil(Math.sqrt(count))

            delegate: Rectangle {
                width: 400
                height: plot.height

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

                    width: parent.width
                    height: 100

                    multipoints: [
                            { color: "red", points: zipN(loadObj.ioHist, loadObj.lowHist, loadObj.mediumHist, loadObj.highHist) },
                            { color: "blue", points: zipN(loadObj.ioHist, loadObj.lowHist, loadObj.mediumHist) },
                            { color: "yellow", points: zipN(loadObj.ioHist, loadObj.lowHist) },
                            { color: "green", points: loadObj.ioHist }
                        ]

                    leftAxisEnabled: true
                    leftAxisTitle: qsTr ("Load, %")
                    yGridEnabled: true

                    minYValue: 0
                    maxYValue: 100

                    alpha: 1
                    background: "transparent"
                    textColor: colorProxy.color_TextBox_TextColor
                }

                Text {
                    id: cpuLabel
                    text: "CPU " + cpuIdx

                    anchors.top: plot.top
                    anchors.horizontalCenter: parent.horizontalCenter

                    color: colorProxy.color_TextBox_TitleTextColor

                    font.bold: true
                }
            }
        }
    }
}
