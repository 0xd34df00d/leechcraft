function CanHandle(link)
{
	return link.indexOf("http://www.3dnews.ru") == 0 ||
			link.indexOf("http://3dnews.ru") == 0;
}

function KeepFirstTag()
{
	return [ 'div[class="newsArticle"]' ];
}