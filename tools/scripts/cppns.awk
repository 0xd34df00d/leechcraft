BEGIN {
	a = 0;
}

$0 == "using namespace LeechCraft::Plugins::" plugin ";" {
	getline;
	getline;
}

$1 == "#include" {
	a = 1;
}

substr($1, 0, 1) == "#" {
	print $0;
}

substr($1, 0, 1) != "#" {
	if (a == 1 && $1 == "")
	{  
		print "";
		print "namespace LeechCraft";
		print "{";
		print "\tnamespace Plugins";
		print "\t{";
		print "\t\tnamespace " plugin;
		print "\t\t{";
		a = 2;
	}
	else if (a == 2)
	{  
			print "\t\t\t" $0;
	}
	else
		print $0;
}

END { 
	print "\t\t};";
	print "\t};";
	print "};";
	print "";
}
