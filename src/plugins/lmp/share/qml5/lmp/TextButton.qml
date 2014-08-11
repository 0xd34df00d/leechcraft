import QtQuick 2.3

Rectangle {
    id: container

    property alias text: label.text

    signal clicked
    signal released

    width: label.width + 20; height: label.height + 6
    smooth: true
    radius: 4

    gradient: Gradient {
        GradientStop { id: gradientStop; position: 0.0; color: palette.light }
        GradientStop { position: 1.0; color: palette.button }
    }

    SystemPalette { id: palette }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: container.clicked()
        onReleased: container.released()
    }

    Text {
        id: label
        anchors.centerIn: parent
        color: palette.buttonText
    }

    states: State {
        name: "pressed"
        when: mouseArea.pressed
        PropertyChanges { target: gradientStop; color: palette.dark }
    }
}
