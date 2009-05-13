for (i = 0; i < document.forms.length; ++i)
{
	for (j = 0; j < document.forms [i].elements.length; ++j)
	{
		type = document.forms [i].elements [j].type;
		result = JSProxy.getFormElement (i,
				document.forms [i].elements [j].name,
				type);
		if (result != null)
		{
			try
			{
				document.forms [i].elements [j].value = result;
			}
			catch (error)
			{
				JSProxy.warning (error);
			}
		}
	}
}
