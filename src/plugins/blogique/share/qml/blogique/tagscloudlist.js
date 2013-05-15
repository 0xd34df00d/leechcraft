var tagsInCloudArray = new Array ();

function addTextTag (tag)
{
	tagsInCloudArray.push (tag);
}

function clearTextTagsArray ()
{
	tagsInCloudArray = [];
}

function getTextTag (tag)
{
	var length = tagsInCloudArray.length;
	for (var i = 0; i < length; ++i)
		if (tagsInCloudArray [i].text == tag)
			return tagsInCloudArray [i];
	return null;
}
