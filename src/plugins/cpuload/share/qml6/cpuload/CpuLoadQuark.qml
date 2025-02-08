import QtQuick
import org.LC.common 1.0

Rectangle {
    id: rootRect

    property real length: cpusView.count * 10
    implicitWidth: viewOrient == "vertical" ? parent.quarkBaseSize : length
    implicitHeight: viewOrient == "vertical" ? length : parent.quarkBaseSize
    width: 100
    height: parent.quarkBaseSize

    color: "transparent"

    Common { id: commonJS }

    Timer {
        interval: CpuLoad_updateInterval
        running: true
        repeat: true

        onTriggered: CpuLoad_proxy.update()
    }

    ListView {
        id: cpusView

        width: parent.length
        height: rootRect.parent.quarkBaseSize
        interactive: false

        orientation: ListView.Horizontal

        model: CpuLoad_model

        transform: Rotation {
            origin.x: 0
            origin.y: 0
            axis { x: 1; y: 1; z: 0 }

            angle: viewOrient == "horizontal" ? 0 : 180
        }

        TimedHoverArea {
            anchors.fill: parent

            property bool closeOnExit: true
            readonly property url tooltipUrl: Qt.resolvedUrl("Tooltip.qml")

            hoverInTimeout: commonHoverInTimeout

            onHoverInTimedOut: {
                commonJS.openTooltip(rootRect,
                        {
                            "loadModel": CpuLoad_model,
                            "cpuProxy": CpuLoad_proxy,
                            "colorProxy": colorProxy,
                            "showIOTime": CpuLoad_showIOTime,
                            "showLowTime": CpuLoad_showLowTime,
                            "showMediumTime": CpuLoad_showMediumTime,
                            "showHighTime": CpuLoad_showHighTime
                        },
                        tooltipUrl);
            }

            function closeTooltip() {
                closeOnExit && commonJS.closeTooltip(tooltipUrl);
            }

            onAreaExited: closeTooltip()

            onReleased: {
                closeOnExit = !closeOnExit;
                closeTooltip();
            }

            hoverEnabled: true
        }

        delegate: Item {
            height: cpusView.height
            width: 10

            Rectangle {
                id: delegateRoot

                transform: [
                    Rotation {
                        origin { x: height / 2; y: height / 2 }
                        angle: -90
                    }
                ]

                height: parent.width
                width: parent.height

                color: "transparent"

                Rectangle {
                    id: highPercRect
                    width: cpusView.height * loadObj.highPercentage
                    height: delegateRoot.height

                    border.width: 1
                    border.color: "red"

                    gradient: Gradient {
                        GradientStop {
                            position: 0
                            color: Qt.lighter("red", 1.5)
                        }
                        GradientStop {
                            position: 1
                            color: Qt.darker("red", 1.5)
                        }
                    }

                    anchors.bottom: delegateRoot.bottom
                }

                Rectangle {
                    id: mediumPercRect
                    width: cpusView.height * loadObj.mediumPercentage
                    height: delegateRoot.height

                    gradient: Gradient {
                        GradientStop {
                            position: 0
                            color: Qt.lighter("blue", 1.5)
                        }
                        GradientStop {
                            position: 1
                            color: Qt.darker("blue", 1.5)
                        }
                    }

                    anchors.left: highPercRect.right
                    anchors.bottom: delegateRoot.bottom
                }

                Rectangle {
                    id: lowPercRect
                    width: cpusView.height * loadObj.lowPercentage
                    height: delegateRoot.height

                    gradient: Gradient {
                        GradientStop {
                            position: 0
                            color: Qt.lighter("yellow", 1.5)
                        }
                        GradientStop {
                            position: 1
                            color: Qt.darker("yellow", 1.5)
                        }
                    }

                    anchors.left: mediumPercRect.right
                    anchors.bottom: delegateRoot.bottom
                }

                Rectangle {
                    id: ioPercRect
                    width: cpusView.height * loadObj.ioPercentage
                    height: delegateRoot.height

                    gradient: Gradient {
                        GradientStop {
                            position: 0
                            color: Qt.lighter("green", 1.5)
                        }
                        GradientStop {
                            position: 1
                            color: Qt.darker("green", 1.5)
                        }
                    }

                    anchors.left: lowPercRect.right
                    anchors.bottom: delegateRoot.bottom
                }
            }
        }
    }
}
