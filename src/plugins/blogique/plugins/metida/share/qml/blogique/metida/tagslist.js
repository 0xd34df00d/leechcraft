var balloonArray = new Array ();
var currentBalloon;

function addBalloon(balloon) {
	balloonArray.push(balloon)
}

function removeBalloon(balloon) {
	var ballon = balloonArray.indexOf(balloon);
	balloonArray.splice (balloonArray.indexOf(balloon), 1);
	balloon.opacity = 0.0;
	balloon.destroy();
}

function getAllBalloones() {
	return balloonArray;
}

function getLastBalloon() {
	return balloonArray [balloonArray.length - 1];
}

function setBalloonAsCurrent(balloon) {
	currentBalloon = balloon;
}

function getCurrentBalloon() {
	return currentBalloon;
}

function count() {
	return balloonArray.length;
}
