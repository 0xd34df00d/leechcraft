import QtQuick 1.1

Rectangle {
    id: rootRect
    width: 400
    height: 300

    smooth: true
    radius: 5

    signal closeRequested()

    property variant svg

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

    Image {
        anchors.top: sensorNameLabel.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        sourceSize.width: width
        sourceSize.height: height

        source: "data:," + svg
        smooth: true
        asynchronous: false
    }
}
