for (i = 0; i < document.forms.length; ++i)
{
	for (j = 0; j < document.forms [i].elements.length; ++j)
	{
		JSProxy.setFormElement (document.location.href,
				i,
				document.forms [i].elements [j].name,
				document.forms [i].elements [j].type,
				document.forms [i].elements [j].value);
	}
}
