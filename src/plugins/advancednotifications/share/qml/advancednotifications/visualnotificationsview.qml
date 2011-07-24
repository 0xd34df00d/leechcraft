import QtQuick 1.0
import "."

Rectangle {
	id: notifArea
	width: 450; height: 200
	smooth: true
	radius: 16
	gradient: Gradient {
		GradientStop { position: 0.0; color: "#CF414141" }
		GradientStop { position: 1.0; color: "#CF1A1A1A" }
	}

	Component {
		id: actionsDelegate

		Rectangle {
			height: actionsListView.height
			width: actionText.width + 5
			smooth: true
			radius: 3
			color: "transparent"

			TextButton {
				id: actionText

				text: model.modelData.actionText
				onClicked: { model.modelData.actionSelected() }
			}
		}
	}

	Component {
		id: eventsDelegate

		Rectangle {
			id: eventRect

			//height: 50
			width: listView.width
			height: (contentsRow.height > 20 ? contentsRow.height : 20) + actionsListView.height + 5
			smooth: true
			radius: 9
			gradient: Gradient {
				GradientStop { position: 0.0; color: "#DF3A3A3A" }
				GradientStop { position: 1.0; color: "#DF101010" }
			}

			Row {
				id: contentsRow

				anchors.left: eventRect.left
				anchors.top: eventRect.top
				anchors.leftMargin: 2
				anchors.topMargin: 2

				Image {
					id: eventPic
					source: image
				}

				Text {
					id: eventText

					text: extendedText
					color: "lightgrey"
					//anchors.left: eventPic.right
					//anchors.leftMargin: 5
				}
			}

			ListView {
				id: actionsListView

				anchors.top: contentsRow.bottom
				anchors.topMargin: 5
				anchors.left: eventRect.left
				anchors.leftMargin: 5
				anchors.bottom: eventRect.bottom
				anchors.bottomMargin: 5

				orientation: ListView.Horizontal

				model: eventActionsModel
				delegate: actionsDelegate
			}
		}
	}
	
	ListView {
		id: listView

		anchors.centerIn: parent
		anchors.topMargin: 5
		width: notifArea.width - 10
		height: notifArea.height
		
		model: eventsModel
		delegate: eventsDelegate
	}
}
