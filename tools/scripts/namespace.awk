BEGIN {
    a = 0;
}

$1 == "#include" {   
    a = 1;
}

$1 == "#endif" {
    a = 0;
    print "\t\t};";
    print "\t};";
    print "};";
    print $0;
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
        if ($1 == "")
        {
            getline;
            if($0 == "#endif")
            {
                a = 0;
                print "\t\t};";
                print "\t};";
                print "};";
                print "";
                print $0;
            }
            else
            {
                print "";
                print "\t\t\t" $0;
            }
        }
        else
            print "\t\t\t" $0;
    }
    else
        print $0;
}

