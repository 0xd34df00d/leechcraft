function CanHandle(link)
{
	return link == "http://habrahabr.ru/rss/";
}

function KeepTags()
{
	return [ 'div[class="content"]' ];
}
