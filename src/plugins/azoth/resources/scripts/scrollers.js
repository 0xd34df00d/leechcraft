"use strict";

function ScrollToBottom() {
	if (window.ShouldScroll)
	    window.scrollTo(0, document.body.scrollHeight);
}
function TestScroll() {
	window.ShouldScroll = document.documentElement.scrollHeight <= (window.innerHeight + window.pageYOffset + window.innerHeight / 5);
}
function InstallEventListeners() {
	window.ShouldScroll = true;
	let scheduleScroll = () => { setTimeout (ScrollToBottom, 0); };
	document.body.addEventListener ("DOMNodeInserted", scheduleScroll, false);
	document.body.addEventListener ("DOMSubtreeModified", scheduleScroll, false);
	window.addEventListener ("resize", scheduleScroll);
	window.addEventListener ("scroll", TestScroll);
}

InstallEventListeners();
ScrollToBottom();
