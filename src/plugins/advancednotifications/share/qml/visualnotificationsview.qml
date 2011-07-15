import QtQuick 1.0

Rectangle {
	id: notificationArea
	width: 300; height: 200
	color: black
	
	Text {
		id: someText
		text: "I'm a notification!"
		y: 20
		anchors.horizontalCenter: page.horizontalCenter
		font.pointSize: 24; font.bold: true
	}
}