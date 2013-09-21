function ScrollToBottom() {
	if (window.ShouldScroll)
		document.body.scrollTop = document.height - window.innerHeight;
}
function TestScroll() {
	window.ShouldScroll = document.height <= (window.innerHeight + window.pageYOffset + window.innerHeight / 5);
}
function InstallEventListeners() {
	window.ShouldScroll = true;
	document.body.addEventListener ("DOMNodeInserted", function () { setTimeout (ScrollToBottom, 0); }, false);
	window.addEventListener ("resize", function () { setTimeout (ScrollToBottom, 0); });
	window.addEventListener ("scroll", TestScroll);
}
