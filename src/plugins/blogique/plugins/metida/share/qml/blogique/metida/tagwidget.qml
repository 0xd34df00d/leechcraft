import QtQuick 1.1
import "tagslist.js" as TagList

Rectangle {
	id: rootRectangle
	width: 300
	height: 300
	radius: 4

	anchors.fill: parent

	property bool inputFieldExsists: false;
	property bool closeButtonHovered: false;
	property Item currentInputField;

	signal tagTextChanged(string text)

	function getTags() {
		var array = TagList.getAllBalloones();
		var length = array.length;
		var tags = Array();
		for (var i = 0; i < length; ++i)
			tags.push (array [i].tag);

		return tags;
	}

	function setTags(tags) {
		var length = tags.length;
		for (var i = 0; i < length; ++i)
		{
			var balloon = tagBalloonComponent.createObject(flowElement)
			balloon.tag = tags[i];
			TagList.addBalloon (balloon);
		}
	}

	function replaceInputWithBalloon(tag) {
		currentInputField.opacity = 0.0;
		currentInputField.destroy ();
		inputFieldExsists = false;

		var balloon = tagBalloonComponent.createObject(flowElement)
		balloon.tag = tag;
		TagList.addBalloon (balloon);

		completerLoader.source = "";

		currentInputField = tagInputComponent.createObject(flowElement);
		inputFieldExsists = true;
	}

	function replaceBalloonWithInput() {
		currentInputField.opacity = 0.0;
		currentInputField.destroy ();
		var lastBalloon = TagList.getLastBalloon ();

		var tag = lastBalloon.tag;
		TagList.removeBalloon(lastBalloon);
		currentInputField = tagInputComponent.createObject(flowElement)
		inputFieldExsists = true;
		currentInputField.text = tag
		currentInputField.lastCursorPosition = tag.length;
		completerLoader.source = "completer.qml"
	}

	function removeInputField () {
		currentInputField.opacity = 0.0;
		currentInputField.destroy();
		inputFieldExsists = false;
		completerLoader.source = "";
	}

	Component {
		id: tagInputComponent

		TextInput{
			id: tagInput
			height: 25
			width: 50
			cursorVisible: true
			focus: true
			property int lastCursorPosition;

			Keys.onReleased: {
				if (event.key == Qt.Key_Escape)
					removeInputField ();
				else if (event.key == Qt.Key_Backspace) {
					if (!lastCursorPosition) {
						if (!TagList.count())
							return;

						replaceBalloonWithInput ();
					}

					lastCursorPosition = cursorPosition;
				}
				else if (event.key == Qt.Key_Down) {
					if (completerLoader.status == Loader.Ready &&
							completerLoader.source != "") {
						completerLoader.item.forceActiveFocus();
					}
				}
				else
					lastCursorPosition = cursorPosition;

				if (cursorPosition &&
						inputFieldExsists &&
						completerLoader.source == "" &&
						tagsModel.count)
					completerLoader.source = "completer.qml"
			}

			onAccepted:
				if (!currentInputField.text == "")
					replaceInputWithBalloon (currentInputField.text);

			onTextChanged: {
				if (text.length * font.pixelSize +5 >= width)
					width += 5

				if (!text.length &&
						completerLoader.status == Loader.Ready)
					completerLoader.source = "";
				else
					tagTextChanged(text)
			}

			onWidthChanged:
				if (completerLoader.status == Loader.Ready)
					completerLoader.item.width = width
		}
	}

	Component {
		id: tagBalloonComponent

		Rectangle {
			id: rect

			property string tag;

			color: "#C9C9C9"
			width: tagName.width + closeImage.width + 13
			height: 25
			radius: 4

			Row {
				id: row
				spacing: 5

				anchors.verticalCenter: rect.verticalCenter
				anchors.left: rect.left
				anchors.leftMargin: 4
				anchors.right: rect.right
				anchors.rightMargin: 4

				Text {
					id: tagName
					text: tag
					color: "blue"
				}

				Image {
					id: closeImage
					smooth: true
					source: "image://ThemeIcons/dialog-close"
					height: tagName.height
					width: tagName.height

					MouseArea {
						id: closeButtonMouseArea
						anchors.fill: parent
						hoverEnabled: true
						onEntered: {
							closeButtonHovered = true;
							TagList.setBalloonAsCurrent (rect);
						}
						onExited:
							closeButtonHovered = false;
						onReleased: {
							TagList.removeBalloon (TagList.getCurrentBalloon ())
							closeButtonHovered = false;
						}
					}
				}
			}
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

		MouseArea {
			id: globalMouseArea
			anchors.fill: flowElement
			enabled: !closeButtonHovered

			onReleased: {
				if (!inputFieldExsists) {
					currentInputField = tagInputComponent.createObject(flowElement)
					inputFieldExsists = true;
				}
			}
		}
	}

	Loader {
		id: completerLoader

		onLoaded: {
			if (!tagsModel.count)
				return;

			item.width = currentInputField.width
			item.tagModel = tagsModel

			item.x = currentInputField.x + flowElement.spacing
			item.y = currentInputField.y + currentInputField.height
		}
	}

	Connections {
		target: completerLoader.item;
		onCurrentItemSelected:
			replaceInputWithBalloon(tag);

		onReturnFocus:
			currentInputField.forceActiveFocus();

		onCloseInput:
			removeInputField();
	}
}
