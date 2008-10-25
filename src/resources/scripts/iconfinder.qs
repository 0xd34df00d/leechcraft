function RequiredClasses ()
{
	return "Dir";
}

function GetOptions ()
{
	var dir = new Dir;
	dir.cd (Settings.ThemesConfigDir);
	var entries = dir.entryList ();
	var result = new Array;
	for (i in entries)
		if (entries [i] != "." && entries [i] != "..")
			result.push (entries [i]);
	return result;
}

function OptionValueToName (name)
{
	return name;
}
