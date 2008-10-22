function RequiredClasses ()
{
	return "Dir";
}

function GetOptions ()
{
	var directory = new Dir;

	directory.cd ("/usr/share/icons");
	var entries = directory.entryInfoList ();

	var result = new String;
	return result;
}

function OptionValueToName (name)
{
	return name;
}
