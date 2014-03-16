import QtQuick 1.1
import org.LC.common 1.0
import org.LC.Ooronee 1.0

Rectangle {
    id: rootRect
    implicitWidth: parent.quarkBaseSize
    implicitHeight: parent.quarkBaseSize

    color: "transparent"

    border.color: "gray"
    border.width: 1
    radius: width / 8

    DropArea {
        anchors.fill: parent
        onTextDropped: console.log(text)
    }
}
