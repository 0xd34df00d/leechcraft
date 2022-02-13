import QtQuick 2.3
import QtQuick.Controls 1.4
import "."

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

    TextButton {
        id: modeButton
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        onReleased: modeState.state = (modeState.state == "topsMode" ? "newsMode" : "topsMode")

        Item {
            id: modeState
            states: [
                State {
                    name: "topsMode"
                    PropertyChanges { target: modeButton; text: newsText }
                    PropertyChanges { target: artistsView; model: topArtistsModel }
                    PropertyChanges { target: hypedTracksView; model: topTracksModel }
                },
                State {
                    name: "newsMode"
                    PropertyChanges { target: modeButton; text: topsText }
                    PropertyChanges { target: artistsView; model: newArtistsModel }
                    PropertyChanges { target: hypedTracksView; model: newTracksModel }
                }
            ]
            state: "topsMode"
        }
    }

    Rectangle {
        id: artistsRect

        anchors.left: parent.left
        anchors.top: modeButton.bottom
        anchors.bottom: parent.bottom
        width: parent.width / 2

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

        Rectangle {
            id: artistsLabel

            color: colorProxy.color_TextView_TopColor

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: Math.max(artistNameLabel.height, modeButton.height)

            Text {
                id: artistNameLabel
                z: 2
                text: artistsLabelText
                font.bold: true
                font.pointSize: 14
                color: colorProxy.color_TextView_TitleTextColor
                anchors.centerIn: parent
            }
        }

        Rectangle {
            z: 2

            anchors.top: artistsLabel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 5

            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: colorProxy.color_TextView_TopColor
                }

                GradientStop {
                    position: 1
                    color: colorProxy.setAlpha(colorProxy.color_TextView_TopColor, 0)
                }
            }
        }

        Rectangle {
            anchors.top: artistsLabel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            clip: true

            SimilarView {
                id: artistsView
                model: topArtistsModel

                anchors.fill: parent
            }
        }
    }

    Rectangle {
        anchors.left: artistsRect.right
        anchors.top: modeButton.bottom
        anchors.bottom: parent.bottom
        anchors.right: parent.right

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

        Rectangle {
            id: tracksLabel

            color: colorProxy.color_TextView_TopColor

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: Math.max(artistNameLabel.height, modeButton.height)

            Text {
                id: tracksNameLabel
                z: 2
                text: tracksLabelText
                font.bold: true
                font.pointSize: 14
                color: colorProxy.color_TextView_TitleTextColor
                anchors.centerIn: parent
            }
        }

        Rectangle {
            z: 2

            anchors.top: tracksLabel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 5

            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: colorProxy.color_TextView_TopColor
                }

                GradientStop {
                    position: 1
                    color: colorProxy.setAlpha(colorProxy.color_TextView_TopColor, 0)
                }
            }
        }

        Rectangle {
            anchors.top: tracksLabel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            clip: true

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

            ScrollView {
                anchors.fill: parent
                style: LMPScrollStyle {}

                ListView {
                    id: hypedTracksView

                    smooth: true

                    model: topTracksModel

                    delegate: Item {
                        height: 150
                        width: hypedTracksView.width
                        smooth: true

                        Rectangle {
                            id: delegateRect

                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            anchors.topMargin: 5
                            anchors.bottomMargin: 5

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

                            border.width: 1
                            border.color: colorProxy.color_TextBox_BorderColor
                            smooth: true

                            Image {
                                id: trackImageThumb
                                width: 62
                                height: 62
                                smooth: true

                                cache: false

                                fillMode: Image.PreserveAspectFit
                                source: thumbImageURL

                                anchors.left: parent.left
                                anchors.leftMargin: 2
                                anchors.top: parent.top
                                anchors.topMargin: 2
                            }

                            Text {
                                id: trackNameLabel
                                text: trackName

                                font.bold: true
                                font.underline: true
                                font.pointSize: 12
                                color: colorProxy.color_TextBox_TitleTextColor
                                anchors.top: parent.top
                                anchors.topMargin: 2
                                anchors.left: trackImageThumb.right
                                anchors.leftMargin: 5
                                anchors.right: parent.right
                                anchors.rightMargin: 8

                                elide: Text.ElideRight

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: stdActions.openLink(trackURL)
                                }
                            }

                            Text {
                                id: trackArtistNameLabel
                                text: artistName

                                font.underline: true
                                font.pointSize: 11
                                color: colorProxy.color_TextBox_TextColor
                                anchors.top: trackNameLabel.bottom
                                anchors.topMargin: 2
                                anchors.left: trackImageThumb.right
                                anchors.leftMargin: 5

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: stdActions.openLink(artistURL)
                                }
                            }

                            BrowseButton {
                                id: browseInfoImage

                                anchors.verticalCenter: trackArtistNameLabel.verticalCenter
                                anchors.left: trackArtistNameLabel.right
                                anchors.leftMargin: 5
                                height: trackArtistNameLabel.height
                                width: height

                                onTriggered: stdActions.browseArtistInfo(artistName)
                            }

                            Text {
                                id: changeLabel
                                text: change
                                color: colorProxy.color_TextBox_Aux1TextColor
                                anchors.top: trackArtistNameLabel.bottom
                                anchors.topMargin: 2
                                anchors.left: trackImageThumb.right
                                anchors.leftMargin: 5
                            }
                        }
                    }
                }
            }
        }
    }
}
