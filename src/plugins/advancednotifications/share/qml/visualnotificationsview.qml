import QtQuick 1.0

Rectangle {
	id: notifArea
	width: 450; height: 200
	smooth: true
	radius: 16
	gradient: Gradient {
		GradientStop { position: 0.0; color: "#CF414141" }
		GradientStop { position: 1.0; color: "#CF1A1A1A" }
	}
	
	ListView {
		id: listView

		anchors.centerIn: parent
		width: notifArea.width - 10
		height: notifArea.height
		
		model: eventsModel
		delegate: Rectangle {
			height: 50
			width: listView.width
			smooth: true
			radius: 9
			gradient: Gradient {
				GradientStop { position: 0.0; color: "#DF3A3A3A" }
				GradientStop { position: 1.0; color: "#DF101010" }
			}
			
			Image {
				id: eventPic
				
				anchors.leftMargin: 2
				anchors.topMargin: 2
				
				source: image
			}
			
			Text {
				id: eventText

				text: extendedText
				color: "grey"
				anchors.left: eventPic.right
				anchors.leftMargin: 5
			}
			
			ListView {
				id: actionsListView

				anchors.right: parent.right
				anchors.rightMargin: 5
				anchors.bottom: parent.bottom
				anchors.bottomMargin: 5
				
				orientation: ListView.Horizontal
				
				model: eventActionsModel
				delegate: Rectangle {
					height: actionsListView.height
					width: 100
					smooth: true
					radius: 3
					
					Text {
						anchors.fill: parent

						text: model.modelData.actionText
					}
				}
			}
		}
	}
}
