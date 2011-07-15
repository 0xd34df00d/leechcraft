import QtQuick 1.0

Rectangle {
	id: notifArea
	width: 450; height: 200
	smooth: true
	radius: 10
	gradient: Gradient {
		GradientStop { position: 0.0; color: "#414141E0" }
		GradientStop { position: 1.0; color: "#1A1A1AC0" }
	}
	
	ListView {
		anchors.centerIn: parent
		
		width: notifArea.width - 10
		height: notifArea.height
		
		model: eventsModel
		delegate: Rectangle {
			anchors.centerIn: parent
			height: 50
			width: 420
			smooth: true
			radius: 6
			gradient: Gradient {
				GradientStop { position: 0.0; color: "#3A3A3AFF" }
				GradientStop { position: 1.0; color: "#101010F2" }
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
				
				anchors.left: eventPic.right
				anchors.leftMargin: 5
			}
			
			ListView {
				anchors.right: parent.right
				anchors.rightMargin: 5
				anchors.bottom: parent.bottom
				anchors.bottomMargin: 5
				
				model: eventActionsModel
				delegate: Rectangle {
					anchors.centerIn: parent
					height: 20
					width: 100
					smooth: true
					radius: 3
					
					Text {
						text: actionText
					}
				}
			}
		}
	}
}