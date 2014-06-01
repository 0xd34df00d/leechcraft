import QtQuick 1.1

Rectangle {
    id: rootRect

    property int singleListWidth: (geometry.width * 2 / 3) / desktopsRepeater.count
    property int maxHeight: geometry.height * 2 / 3
    property int maxChildrenHeight: maxHeight

    Component.onCompleted: {
        var res = 0;
        for (var i = 0; i < desktopsRepeater.count; ++i)
            res = Math.max(res, desktopsRepeater.itemAt(i).contentHeight);
        rootRect.maxChildrenHeight = res;
    }

    width: desktopsRepeater.count * singleListWidth
    height: Math.min(maxHeight, maxChildrenHeight)

    radius: 3
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

    Row {
        id: desktopsRow
        anchors.fill: parent

        Repeater {
            id: desktopsRepeater
            model: desktopsModel

            Item {
                width: rootRect.singleListWidth
                height: rootRect.height
                property real contentHeight: flickableList.contentHeight + desktopNameLabel.height + flickableList.anchors.topMargin

                Text {
                    id: desktopNameLabel
                    text: desktopName

                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter

                    color: colorProxy.color_TextBox_TitleTextColor
                    font.underline: isCurrent
                    font.bold: isCurrent
                    font.pointSize: 16

                    MouseArea {
                        anchors.fill: parent
                        onClicked: pagerProxy.showDesktop(desktopID)
                    }
                }

                Flickable {
                    id: flickableList
                    anchors.top: desktopNameLabel.bottom
                    anchors.topMargin: 5
                    width: rootRect.singleListWidth
                    height: parent.height - desktopNameLabel.height
                    contentHeight: subModelColumn.height
                    contentWidth: subModelColumn.width
                    clip: true
                    Column {
                        id: subModelColumn

                        spacing: 2

                        width: rootRect.singleListWidth
                        Component.onCompleted: {
                            var res = 0;
                            for (var i = 0; i < subModelRepeater.count; ++i)
                                res += subModelRepeater.itemAt(i).height;
                            if (subModelRepeater.count)
                                res += (subModelRepeater.count - 1) * subModelColumn.spacing;
                            subModelColumn.height = res;
                        }

                        Repeater {
                            id: subModelRepeater

                            model: subModel

                            Rectangle {
                                id: appRectangle

                                width: rootRect.singleListWidth
                                height: winIconImage.height + (winSnapImage.visible ? winSnapImage.height : 0) +
                                        winSnapImage.anchors.topMargin + winSnapImage.anchors.bottomMargin

                                border.width: 1
                                border.color: colorProxy.color_TextBox_BorderColor

                                radius: 5
                                smooth: true

                                gradient: Gradient {
                                    GradientStop {
                                        id: topHighlightGradient
                                        position: 0
                                        color: colorProxy.color_TextBox_TopColor
                                    }
                                    GradientStop {
                                        id: bottomHighlightGradient
                                        position: 1
                                        color: colorProxy.color_TextBox_BottomColor
                                    }
                                }

                                states: [
                                    State {
                                        name: "highlight"
                                        when: isActive
                                        PropertyChanges {
                                            target: topHighlightGradient
                                            color: colorProxy.color_TextBox_HighlightTopColor
                                        }
                                        PropertyChanges {
                                            target: bottomHighlightGradient
                                            color: colorProxy.color_TextBox_HighlightBottomColor
                                        }
                                        PropertyChanges {
                                            target: appRectangle
                                            border.color: colorProxy.color_TextBox_HighlightBorderColor
                                        }
                                    }
                                ]

                                transitions: [
                                    Transition {
                                        from: ""
                                        to: "highlight"
                                        reversible: true
                                        PropertyAnimation { properties: "color"; duration: 350 }
                                    }
                                ]

                                Image {
                                    id: winIconImage
                                    source: "image://WinIcons/" + wid + '/' + width
                                    width: showThumbs ? 64 : 32
                                    height: width
                                    anchors.top: parent.top
                                    anchors.topMargin: 2
                                    anchors.left: parent.left
                                    anchors.leftMargin: 2
                                    cache: false
                                }

                                Text {
                                    id: appNameLabel

                                    anchors.left: winIconImage.right
                                    anchors.leftMargin: 2
                                    anchors.right: parent.right
                                    anchors.rightMargin: 2
                                    anchors.verticalCenter: winIconImage.verticalCenter

                                    text: winName
                                    elide: Text.ElideRight

                                    color: colorProxy.color_TextBox_TitleTextColor
                                }

                                Image {
                                    id: winSnapImage

                                    source: "image://WinSnaps/" + wid
                                    anchors.top: winIconImage.bottom
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.margins: 5

                                    smooth: true
                                    fillMode: Image.PreserveAspectFit
                                    cache: false

                                    visible: showThumbs
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: pagerProxy.showWindow(wid)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
