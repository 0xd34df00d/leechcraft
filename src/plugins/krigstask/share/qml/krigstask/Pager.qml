import QtQuick 1.1

Rectangle {
    id: rootRect

    property int singleListWidth: 200
    property int maxHeight: 500
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
                    font.underline: true
                    font.pointSize: 16
                }

                Flickable {
                    id: flickableList
                    anchors.top: desktopNameLabel.bottom
                    anchors.topMargin: 5
                    width: rootRect.singleListWidth
                    height: parent.height - desktopNameLabel.height
                    contentHeight: subModelColumn.height
                    contentWidth: subModelColumn.width
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
                                width: rootRect.singleListWidth
                                height: winIconImage.height + winSnapImage.height + winSnapImage.anchors.topMargin + winSnapImage.anchors.bottomMargin

                                border.width: 1
                                border.color: colorProxy.color_TextBox_BorderColor

                                radius: 5

                                gradient: Gradient {
                                    GradientStop {
                                        position: 0
                                        color: colorProxy.color_TextBox_TopColor
                                    }
                                    GradientStop {
                                        position: 1
                                        color: colorProxy.color_TextBox_BottomColor
                                    }
                                }

                                Image {
                                    id: winIconImage
                                    source: "image://WinIcons/" + wid + '/' + width
                                    width: 64
                                    height: 64
                                    anchors.top: parent.top
                                    anchors.left: parent.left
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
                                    font.pointSize: 14
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
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
