import QtQuick 1.1
import org.LC.common 1.0

Rectangle {
    id: rootRect
    width: 400
    height: 100 * loadView.count

    smooth: true
    radius: 5

    signal closeRequested()

    function beforeDelete() {}

    property alias isHovered: wholeArea.containsMouse

    MouseArea {
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

    ListView {
        id: loadView

        model: loadModel

        anchors.fill: parent

        delegate: Item {
            width: 600
            height: 100

            Plot {
                anchors.fill: parent

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
            }
        }
    }
}
