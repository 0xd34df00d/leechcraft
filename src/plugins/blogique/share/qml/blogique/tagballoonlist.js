var balloonArray = new Array ();
var currentBalloon;

function addBalloon (balloon)
{
	balloonArray.push (balloon)
}

function removeBalloon (balloon)
{
	var ballon = balloonArray.indexOf (balloon);
	balloonArray.splice (balloonArray.indexOf (balloon), 1);
	balloon.opacity = 0.0;
	balloon.destroy ();
}

function clearBalloonsArray ()
{
	var length = balloonArray.length;
	for (var i = 0; i < length; ++i)
		balloonArray [i].destroy ();
	balloonArray.length = 0;
}

function getAllBalloones ()
{
	return balloonArray;
}

function getLastBalloon ()
{
	return balloonArray [balloonArray.length - 1];
}

function setBalloonAsCurrent (balloon)
{
	currentBalloon = balloon;
}

function getCurrentBalloon ()
{
	return currentBalloon;
}

function count ()
{
	return balloonArray.length;
}

function contains (tagName)
{
	var length = balloonArray.length;
	for (var i = 0; i < length; ++i)
		if (balloonArray [i].tag == tagName)
			return true;

	return false;
}

function getBalloonByName (tagName)
{
	var length = balloonArray.length;
	for (var i = 0; i < length; ++i)
		if (balloonArray [i].tag == tagName)
			return balloonArray [i];
	return null;
}

function calculateContentHeight (width, spacing)
{
	var length = balloonArray.length;
	var lineLength = 0;
	var result = spacing;
	var tagsInLine = 0;
	for (var i = 0; i < length; ++i)
	{
		var diffSpacing = tagsInLine >= 1 ? spacing : 0;
		if (i == length - 1 ||
			lineLength + balloonArray [i].width + diffSpacing > width)
		{
			result += balloonArray [i].height + spacing;
			lineLength = 0;
			tagsInLine = 0;
		}

		lineLength += balloonArray [i].width;
		lineLength += diffSpacing;
		tagsInLine++;
	}

	if (length > 0)
		result += 2 * (balloonArray [0].height + spacing);

	console.log (result, width)
	return result;
}

function getOffset (contentHeight, spacing)
{
	var ht = contentHeight;
	ht -= 2 * (balloonArray [0].height + spacing);

	return ht;
}