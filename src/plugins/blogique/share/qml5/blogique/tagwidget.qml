import QtQuick 2.3
import org.LC.common 1.0
import "tagballoonlist.js" as TagBalloonList

Rectangle
{
	id: rootRectangle

	anchors.fill: parent

	gradient: Gradient
	{
		GradientStop
		{
			position: 0
			color: colorProxy.color_TextView_TopColor
		}
		GradientStop
		{
			position: 1
			color: colorProxy.color_TextView_BottomColor
		}
	}

	property bool inputFieldExsists: false;
	property Item currentInputField;

	signal tagTextChanged (string text);
	signal tagRemoved (string tag);
	signal tagAdded (string tag);

	function getTags ()
	{
		var array = TagBalloonList.getAllBalloones ();
		var length = array.length;
		var tags = Array ();
		for (var i = 0; i < length; ++i)
			tags.push (array [i].tag);

		return tags;
	}

	function updateFlickableHeight ()
	{
		var res = TagBalloonList.calculateContentHeight (flowElement.spacing, flowElement.height);
		flickable.contentHeight = res;
	}

	function setTags (tags)
	{
		TagBalloonList.clearBalloonsArray ();
		var length = tags.length;
		for (var i = 0; i < length; ++i)
			TagBalloonList.addBalloon (createBalloonObject (tags [i]));
		updateFlickableHeight ();
	}

	function replaceInputWithBalloon (tag)
	{
		removeInputField ();
		TagBalloonList.addBalloon (createBalloonObject (tag));
		tagAdded (tag)
		createInputFieldObject ();
		flickable.contentY = TagBalloonList.getOffset ();
	}

	function replaceBalloonWithInput ()
	{
		removeInputField ();

		var lastBalloon = TagBalloonList.getLastBalloon ();
		var tag = lastBalloon.tag;
		TagBalloonList.removeBalloon (lastBalloon);
		updateFlickableHeight ();
		flickable.contentY = TagBalloonList.getOffset ();
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
		updateFlickableHeight ();
	}

	Component
	{
		id: tagInputComponent

		TextInput
		{
			id: tagInput
			height: 25
			width: 10
			cursorVisible: true
			focus: true
			color: colorProxy.color_TextView_TitleTextColor

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

			color: colorProxy.color_TextBox_TopColor
			width: tagName.width + 13 + closeImage.width
			height: 25
			radius: 4
			smooth: true

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
						updateFlickableHeight ();
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
					color: colorProxy.color_TextBox_TitleTextColor
				}

				ActionButton
				{
					id: closeImage
					smooth: true
					height: rect.height * 2 / 3
					width: rect.height * 2 / 3
					actionIconURL: "image://ThemeIcons/dialog-close"
					onTriggered: rect.removeBegin = true;
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
		contentHeight: flickable.height;

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
		
		onWidthChanged:
			updateFlickableHeight ();
		onHeightChanged:
			updateFlickableHeight ();
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
				updateFlickableHeight ();
				flickable.contentY = TagBalloonList.getOffset ();
			}
			else
			{
				var balloon = TagBalloonList.getBalloonByName (tag);
				balloon.removeBegin = true;
			}
		}
	}
}
