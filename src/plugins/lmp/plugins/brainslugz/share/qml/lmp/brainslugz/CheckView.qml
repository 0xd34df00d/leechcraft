import QtQuick 1.1
import Effects 1.0

Rectangle {
    id: rootRect

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

    ListView {
        anchors.fill: parent
    }
}

