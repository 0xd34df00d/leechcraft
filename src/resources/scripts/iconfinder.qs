function RequiredClasses ()
{
	return "Dir";
}

function GetIconPacks (baseDir)
{
	var packs = new Array;
	var directory = new Dir;
	directory.cd (baseDir);

	var entries = directory.entryList ();

	for (i in entries)
	{
		if (entries [i] == "." || entries [i] == "..")
			continue;

		var iconPackDirectory = new Dir;
		iconPackDirectory.cd (baseDir);
		iconPackDirectory.cd (entries [i]);

		var sizes = iconPackDirectory.entryList ();
		for (j in sizes)
		{
			if (sizes [j] == "." || sizes [j] == "..")
				continue;

			var iconContainer = new Dir;
			iconContainer.cd (baseDir);
			iconContainer.cd (entries [i]);
			iconContainer.cd (sizes [j]);

			var types = iconContainer.entryList ();
			for (k in types)
			{
				if (types [k] != "actions")
					continue;

				var iconSizePack = new Dir;
				iconSizePack.cd (baseDir);
				iconSizePack.cd (entries [i]);
				iconSizePack.cd (sizes [j]);
				iconSizePack.cd (types [k]);

				var filter = new Array;
				filter [0] = "lc_*";
				var icons = iconSizePack.entryList (filter);

				if (icons.pop () != null)
				{
					var found = false;
					for (p in packs)
					{
						if (packs [p] == entries [i])
						{
							found = true;
							break;
						}
					}
					if (!found)
					{
						packs.push (entries [i]);
					}
					break;
				}
			}
		}
	}

	packs.sort ();
	return packs;
}

function GetOptions ()
{
	var dir = new Dir;

	var options = new Array ();
	options [0] = GetIconPacks ("/usr/share/icons");
	options [1] = GetIconPacks ("/usr/local/share/icons");
	options [2] = GetIconPacks ("~/.icons");

	var result = new Array ();
	for (i in options)
		for (j in options [i])
			result.push (options [i] [j]);

	return result;
}

function OptionValueToName (name)
{
	return name;
}
