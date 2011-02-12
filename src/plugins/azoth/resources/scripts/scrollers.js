function ScrollToBottom() {
	if (window.ShouldScroll)
		window.scroll(0, document.height - window.innerHeight + 20);
}
function TestScroll() {
	window.ShouldScroll = document.height <= (window.innerHeight + window.pageYOffset);
}
function InstallEventListeners() {
	window.ShouldScroll = true;
	document.body.addEventListener ("DOMNodeInserted", ScrollToBottom, false);
	window.onresize = function() { setTimeout(ScrollToBottom, 0); };
	window.addEventListener ("scroll", TestScroll);
}