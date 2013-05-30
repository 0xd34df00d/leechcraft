import QtQuick 1.1
import "tagballoonlist.js" as TagBalloonList

Rectangle
{
	id: rootRectangle

	anchors.fill: parent

	property bool inputFieldExsists: false;
	property Item currentInputField;

	signal tagTextChanged (string text);
	signal tagRemoved (string tag);

	function getTags ()
	{
		var array = TagBalloonList.getAllBalloones ();
		var length = array.length;
		var tags = Array ();
		for (var i = 0; i < length; ++i)
			tags.push (array [i].tag);

		return tags;
	}

	function setTags (tags)
	{
		var length = tags.length;
		for (var i = 0; i < length; ++i)
			TagBalloonList.addBalloon (createBalloonObject (tags [i]));
	}

	function replaceInputWithBalloon (tag)
	{
		removeInputField ();
		TagBalloonList.addBalloon (createBalloonObject (tag));
		createInputFieldObject ();
	}

	function replaceBalloonWithInput ()
	{
		removeInputField ();

		var lastBalloon = TagBalloonList.getLastBalloon ();
		var tag = lastBalloon.tag;
		TagBalloonList.removeBalloon (lastBalloon);
		tagRemoved (tag);

		createInputFieldObject ()
		currentInputField.text = tag
		currentInputField.lastCursorPosition = tag.length;
		completerLoader.source = "completer.qml"
	}

	function removeInputField ()
	{
		currentInputField.opacity = 0.0;
		currentInputField.destroy ();
		inputFieldExsists = false;
		completerLoader.source = "";
	}

	function createBalloonObject (tagName)
	{
		return tagBalloonComponent.createObject (flowElement, { tag: tagName });
	}

	function createInputFieldObject ()
	{
		currentInputField = tagInputComponent.createObject (flowElement);
		inputFieldExsists = true;
	}

	Component
	{
		id: tagInputComponent

		TextInput
		{
			id: tagInput
			height: 25
			width: 50
			cursorVisible: true
			focus: true

			property int lastCursorPosition;

			Keys.onReleased:
			{
				if (event.key == Qt.Key_Escape)
					removeInputField ();
				else if (event.key == Qt.Key_Backspace)
				{
					if (!lastCursorPosition)
					{
						if (!TagBalloonList.count ())
							return;

						replaceBalloonWithInput ();
					}

					lastCursorPosition = cursorPosition;
				}
				else if (event.key == Qt.Key_Down)
				{
					if (completerLoader.status == Loader.Ready &&
							completerLoader.source != "")
						completerLoader.item.forceActiveFocus ();
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

			onTextChanged:
			{
				if (text.length * font.pixelSize +5 >= width)
					width += 5

				if (!text.length &&
						completerLoader.status == Loader.Ready)
					completerLoader.source = "";
				else
					tagTextChanged (text)
			}

			onWidthChanged:
				if (completerLoader.status == Loader.Ready)
					completerLoader.item.width = width
		}
	}

	Component
	{
		id: tagBalloonComponent

		Rectangle
		{
			id: rect

			property string tag;
			property bool removeBegin;

			color: "#C9C9C9"
			width: tagName.width + closeImage.width + 13
			height: 25
			radius: 4

			NumberAnimation
			{
				id:removeAnimation
				target:rect
				properties:"scale"
				to:0
				duration:500
				running: rect.removeBegin;
				onRunningChanged:
					if (running == false)
					{
						TagBalloonList.removeBalloon (rect)
						tagRemoved (tag)
					}
			}

			Row
			{
				id: row
				spacing: 5

				anchors.verticalCenter: rect.verticalCenter
				anchors.left: rect.left
				anchors.leftMargin: 4
				anchors.right: rect.right
				anchors.rightMargin: 4

				Text
				{
					id: tagName
					text: tag
					color: "blue"
				}

				Image
				{
					id: closeImage
					smooth: true
					height: tagName.height
					width: tagName.height
					source: "image://ThemeIcons/dialog-close/" + width

					MouseArea
					{
						id: closeButtonMouseArea
						anchors.fill: parent
						hoverEnabled: true
						onEntered: TagBalloonList.setBalloonAsCurrent (rect);
						onReleased: rect.removeBegin = true;
					}
				}
			}
		}
	}

	Flickable
	{
		id: flickable
		clip: true
		boundsBehavior: Flickable.StopAtBounds

		anchors.fill: parent
		anchors.leftMargin: 10
		anchors.rightMargin: 10
		contentWidth: flickable.width;
		contentHeight: flickable.height * 4;

		Flow
		{
			id: flowElement
			anchors.fill: parent
			anchors.topMargin: 4
			anchors.bottomMargin: 4
			spacing: 10
		}

		MouseArea
		{
			id: globalMouseArea
			anchors.fill: flowElement
			z: flowElement.z - 1

			onReleased:
				if (!inputFieldExsists)
					createInputFieldObject ()
		}
	}

	Loader
	{
		id: completerLoader

		onLoaded:
		{
			if (!tagsModel.count)
				return;

			item.width = currentInputField.width
			item.tagModel = tagsModel

			item.x = currentInputField.x + flowElement.spacing
			item.y = currentInputField.y + currentInputField.height
		}
	}

	Connections
	{
		target: completerLoader.item;

		onCurrentItemSelected: replaceInputWithBalloon (tag);
		onReturnFocus: currentInputField.forceActiveFocus ();
		onCloseInput: removeInputField ();
	}

	Connections
	{
		target: mainWidget
		onTagSelected:
		{
			if (!TagBalloonList.contains (tag))
			{
				if (currentInputField)
					removeInputField ();

				TagBalloonList.addBalloon (createBalloonObject (tag));
			}
			else
			{
				var balloon = TagBalloonList.getBalloonByName (tag);
				balloon.removeBegin = true;
			}
		}
	}
}
