var tagsInCloudArray = new Array ();

function addTextTag (tag)
{
	tagsInCloudArray.push (tag);
}

function clearTextTagsArray ()
{
	var length = tagsInCloudArray.length;
	for (var i = 0; i < length; ++i)
		tagsInCloudArray [i].destroy ();
	tagsInCloudArray.length = 0;
}

function unselectAllTags ()
{
	var length = tagsInCloudArray.length;
	for (var i = 0; i < length; ++i)
		tagsInCloudArray [i].font.strikeout = false;
}

function getTextTag (tag)
{
	var length = tagsInCloudArray.length;
	for (var i = 0; i < length; ++i)
		if (tagsInCloudArray [i].text == tag)
			return tagsInCloudArray [i];
	return null;
}

function calculateContentHeight (width, spacing)
{
	var length = tagsInCloudArray.length;
	var maxHeight = 0;
	var lineLength = 0;
	var result = spacing;
	var tagsInLine = 0;
	for (var i = 0; i < length; ++i)
	{
		var diffSpacing = tagsInLine >= 1 ? spacing : 0;
		if (i == length - 1 ||
				lineLength + tagsInCloudArray [i].width + diffSpacing > width)
		{
			result += (maxHeight ? maxHeight : tagsInCloudArray [i].height) + spacing;
			lineLength = 0;
			maxHeight = 0;
			tagsInLine = 0;
		}

		if (tagsInCloudArray [i].height > maxHeight)
			maxHeight = tagsInCloudArray [i].height;

		lineLength += tagsInCloudArray [i].width;
		lineLength += diffSpacing;
		tagsInLine++;
	}
	return result;
}
