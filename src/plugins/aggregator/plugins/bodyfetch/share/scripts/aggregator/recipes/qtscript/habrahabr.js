function CanHandle(link)
{
	return link.indexOf("http://habrahabr.ru/rss/") == 0;
}

function KeepFirstTag()
{
	return [ 'div[class="content"]' ];
}
