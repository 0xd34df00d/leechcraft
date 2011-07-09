function CanHandle(link)
{
	return link.indexOf("http://www.opennet.ru/opennews") == 0;
}

function KeepFirstTag()
{
	return [ 'td[class="chtext"] p' ];
}