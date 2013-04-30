function ScrollToBottom() {
	if (window.ShouldScroll)
		window.scroll(0, document.height - window.innerHeight + 36);
}
function TestScroll() {
	window.ShouldScroll = document.height <= (window.innerHeight + window.pageYOffset + 36);
}
function InstallEventListeners() {
	window.ShouldScroll = true;
	document.body.addEventListener ("DOMNodeInserted", function () { setTimeout (ScrollToBottom, 0); }, false);
	window.onresize = ScrollToBottom;
	window.addEventListener ("scroll", TestScroll);
}
