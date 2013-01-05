import QtQuick 1.1
import "../common/"

Rectangle {
    id: rootRect

    signal closeRequested()
    signal moveRequested(int from, int to)
}
