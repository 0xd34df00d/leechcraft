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

function getTextTag (tag)
{
	var length = tagsInCloudArray.length;
	for (var i = 0; i < length; ++i)
		if (tagsInCloudArray [i].text == tag)
			return tagsInCloudArray [i];
	return null;
}
