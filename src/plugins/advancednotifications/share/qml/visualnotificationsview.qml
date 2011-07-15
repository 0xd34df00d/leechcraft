import QtQuick 1.0

Rectangle {
	id: notifArea
	width: 450; height: 200
	radius: 4
	gradient: Gradient {
		GradientStop { position: 0.0; color: "#41414133" }
		GradientStop { position: 1.0; color: "#1A1A1A22" }
	}
	
	ListView {
		anchors.centerIn: parent
		width: notifArea.width - 10
		height: notifArea.height
		
		model: eventsModel
		delegate: Rectangle {
			height: 50
			width: 420
			radius: 2
			gradient: Gradient {
				GradientStop { position: 0.0; color: "#3A3A3A33" }
				GradientStop { position: 1.0; color: "#10101022" }
			}
			
			Image {
				source: image
			}
			
			Text {
				text: extendedText
			}
		}
	}
}