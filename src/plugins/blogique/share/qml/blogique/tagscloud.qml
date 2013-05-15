import QtQuick 1.1
import Effects 1.0

Rectangle {
	id: rootRect
	anchors.fill: parent;

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

	function tagsUpdated (tags)
	{
		for (var prop in tags) {
			console.log (prop, '=', tags [prop])
		}
	}

	Flickable {
		id: flickable
		clip: true
		boundsBehavior: Flickable.StopAtBounds

		anchors.fill: parent
		anchors.leftMargin: 10
		anchors.rightMargin: 10
		contentWidth: flickable.width;
		contentHeight: flickable.height * 4;

		Flow {
			id: flowElement
			anchors.fill: parent
			anchors.topMargin: 4
			anchors.bottomMargin: 4
			spacing: 10
		}
	}

	Component.onCompleted: {
		for (var prop in tags) {
			console.log (prop, '=', tags [prop])
		}
	}
}