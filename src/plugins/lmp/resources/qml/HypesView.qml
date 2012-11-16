import QtQuick 1.0
import Effects 1.0
import "."

Rectangle {
    id: rootRect
    anchors.fill: parent

    color: "#000000"

    signal linkActivated(string id)

    Rectangle {
        id: artistsRect

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width / 2

        color: "#000000"

        Rectangle {
            id: artistsLabel

            color: "#53485F"

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
                color: "#dddddd"
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
                    color: "#ff53485F"
                }

                GradientStop {
                    position: 1
                    color: "#0053485F"
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

                onLinkActivated: rootRect.linkActivated(id)
            }
        }
    }

    Rectangle {
        anchors.left: artistsRect.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        color: "#000000"

        Rectangle {
            id: tracksLabel

            color: "#53485F"

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
                color: "#dddddd"
                anchors.centerIn: parent
            }

            TextButton {
                id: modeButton
                anchors.top: parent.top
                anchors.right: parent.right
                onReleased: { console.log(modeState.state); modeState.state = (modeState.state == "topsMode" ? "newsMode" : "topsMode"); }

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
                    color: "#ff53485F"
                }

                GradientStop {
                    position: 1
                    color: "#0053485F"
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
                    color: "#53485F"
                }

                GradientStop {
                    position: 1
                    color: "#222222"
                }
            }

            ListView {
                id: hypedTracksView

                anchors.fill: parent
                smooth: true

                model: topTracksModel

                delegate: Item {
                    height: 75
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
                                position: 1
                                color: "#42394b"
                            }

                            GradientStop {
                                position: 0
                                color: "#000000"
                            }
                        }

                        border.width: 1
                        border.color: "#000000"
                        smooth: true

                        Image {
                            id: trackImageThumb
                            width: 62
                            height: 62
                            smooth: true

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
                            color: "#dddddd"
                            anchors.top: parent.top
                            anchors.topMargin: 2
                            anchors.left: trackImageThumb.right
                            anchors.leftMargin: 5

                            MouseArea {
                                anchors.fill: parent
                                onClicked: rootRect.linkActivated(trackURL)
                            }
                        }

                        Text {
                            id: trackArtistNameLabel
                            text: artistName

                            font.underline: true
                            font.pointSize: 11
                            color: "#bbbbbb"
                            anchors.top: trackNameLabel.bottom
                            anchors.topMargin: 2
                            anchors.left: trackImageThumb.right
                            anchors.leftMargin: 5

                            MouseArea {
                                anchors.fill: parent
                                onClicked: rootRect.linkActivated(artistURL)
                            }
                        }

                        Text {
                            id: artistTagsLabel
                            text: change
                            color: "#888888"
                            anchors.top: parent.top
                            anchors.topMargin: 2
                            anchors.right: parent.right
                            anchors.rightMargin: 2
                        }
                    }
                }
            }
        }
    }
}
