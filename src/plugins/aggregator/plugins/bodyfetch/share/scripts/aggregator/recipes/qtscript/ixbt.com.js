function CanHandle(link)
{
	return link.indexOf("http://www.ixbt.com") == 0;
}

function KeepFirstTag()
{
	// handle both news (first selector) and full articles (second one)
	return [ 'div[class="news_body"]', 'div[class="rebrand_article_content_block"]' ];
}