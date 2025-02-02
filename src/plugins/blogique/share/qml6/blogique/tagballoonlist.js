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

function calculateContentHeight (spacing, defaultHeight)
{
	var height = defaultHeight;
	if (balloonArray.length != 0)
	{
		var balloon = balloonArray [balloonArray.length - 1];
		height = balloon.y + (balloon.height + spacing) * 2;
	}
	return height;
}

function getOffset ()
{
	var offset = 0;
	if (balloonArray.length != 0)
		offset = balloonArray [balloonArray.length - 1].y;
	return offset;
}
