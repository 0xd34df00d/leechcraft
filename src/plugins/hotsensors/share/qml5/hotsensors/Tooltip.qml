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

        Plot {
            anchors.fill: parent

            points: pointsList

            plotTitle: sensorName

            minYValue: 0
            maxYValue: Math.max(maxTemp, critTemp)
            minXValue: 0
            maxXValue: maxPointsCount

            leftAxisEnabled: true
            leftAxisTitle: qsTr ("Temperature, °C")
            yGridEnabled: true
            yMinorGridEnabled: true

            alpha: 0.4
            background: "transparent"
            textColor: colorProxy.color_TextBox_TextColor
            gridLinesColor: colorProxy.color_TextBox_Aux2TextColor
        }
    }
}
