import QtQuick 2.3
import "tagscloudlist.js" as TagsCloudList

Rectangle
{
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

	signal tagSelected (string tag);

	Flickable
	{
		id: flickable
		clip: true
		boundsBehavior: Flickable.StopAtBounds

		anchors.fill: parent
		anchors.leftMargin: 10
		anchors.rightMargin: 10
		contentWidth: flickable.width;
		contentHeight: TagsCloudList.calculateContentHeight(flowElement.width, flowElement.spacing);

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
		}
		
		onWidthChanged:
			updateTagsCloud ();
		onHeightChanged:
			updateTagsCloud ();
	}

	Component
	{
		id: tagComponent

		Text
		{
			property string tag;
			property real pointSz;

			id: tagText
			text: tag;

			color: colorProxy.color_TextBox_TextColor

			font.pointSize: pointSz;
			font.underline: !tagMouseArea.containsMouse

			MouseArea
			{
				id: tagMouseArea;
				hoverEnabled: true;
				anchors.fill: parent

				onReleased:
				{
					tagSelected (tag);
					tagText.font.strikeout = !tagText.font.strikeout;
				}
			}
		}
	}

	function clearTags ()
	{
		TagsCloudList.clearTextTagsArray ();
		updateTagsCloud ()
	}

	function addTag (tag, count, maxCount)
	{
		var freq = 11 + count / maxCount * 30;
		var obj = tagComponent.createObject (flowElement, {tag: tag, pointSz: freq});
		TagsCloudList.addTextTag (obj)
		updateTagsCloud ()
	}

	function selectTag (tag, select)
	{
		var obj = TagsCloudList.getTextTag (tag);
		if (obj)
			obj.font.strikeout = select;
	}

	function setTags (tags)
	{
		TagsCloudList.unselectAllTags ();

		var length = tags.length;
		for (var i = 0; i < length; ++i)
			selectTag (tags [i], true)
	}

	function updateTagsCloud ()
	{
		flickable.contentHeight = TagsCloudList.calculateContentHeight(flowElement.width, flowElement.spacing)
	}
}
