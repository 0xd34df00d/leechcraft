BEGIN {
    a = 0;
}

$1 == "#include" {   
    print $0;
    a = 1;
}

substr($1, 0, 1) != "#" {   
    if (a == 1 && $1 == "")
    {   
        print "";
        print "using namespace LeechCraft::Plugins::" plugin ";";
        print "";
        a = 2;
    }
    else if (a == 2)
    {   
        if ($1 == "")
        {
            getline;
			print "";
			print $0;
        }
        else
            print $0;
    }
    else
        print $0;
}

