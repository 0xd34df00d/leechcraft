import QtQuick 2.3
import QtQuick.Window 2.1
import org.LC.common 1.0

Window {
    width: 400
    height: 300

    flags: Qt.ToolTip

    Rectangle {
        id: rootRect

        anchors.fill: parent

        smooth: true
        radius: 5

        property variant pointsList

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

        Text {
            id: sensorNameLabel
            text: sensorName

            anchors.top: parent.top
            anchors.topMargin: 5
            anchors.horizontalCenter: parent.horizontalCenter

            color: colorProxy.color_TextView_TitleTextColor
            font.bold: true
        }

        Plot {
            anchors.top: sensorNameLabel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            points: pointsList

            minYValue: 0
            maxYValue: Math.max(maxTemp, critTemp)

            leftAxisEnabled: true
            leftAxisTitle: qsTr ("Temperature, °C")
            yGridEnabled: true
        }
    }
}
