function CanHandle(link)
{
	return link.indexOf("http://www.opennet.ru/opennews") == 0;
}

function KeepFirstTagInnerXml()
{
	return [ 'td[class="chtext"]' ];
}