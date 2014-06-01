#!/usr/bin/env python
# This script replaces the path of the dependencies in all the dylib files in the specified 
# directory. The paths must match a given user string and this string will be replaced by the
# given replacement string. For example:
# replacepath.py --old /opt/local/lib --new @executable_path/Contents/lib --dir ./libs -Rs
# Add the -R switch to enable recursive exploration of input directory.
# By Andres Colubri

import sys
import os
import subprocess

def print_help():
    print 'This script replaces the path of the dependencies in all the dylib files in the '
    print 'specified directory. The paths must match a given user string and this string   '
    print 'will be replaced by the given replacement string. For example:                  '   
    print 'replacepath.py --old /opt/local/lib/ --new @executable_path/ --dir ./libs -R    '
    print 'Add the -R switch to enable recursive exploration of input directory.           ' 

def print_error(error):
    print error + '. Get some help by typing:'
    print 'replacepath.py --help'

def process_file(old, new, fullname, fn):
    print 'Replacing dependencies in', fn
    
    ext = os.path.splitext(fn)[1]
    
    # Last occurrence of '-' (lib-test-0.10.dylib):
    n0 = fullname.rfind('-')
    # First occurrence of '.' (lib.1.2.dylib):
    n1 = fullname.find('.')
    if n0 == -1: n0 = 10000
    if n1 == -1: n1 = 10000
    n = min(n0, n1)
    name = fullname[0:n]
    
    if os.path.islink(fn):
        # Skipping symbolic links
        print '  File is a symlink... skipping.'
        return
        
    id_changed = 0     
    
    pipe = subprocess.Popen('otool -L ' + fn, shell=True, stdout=subprocess.PIPE).stdout
    output = pipe.readlines()

    for line in output[1:len(output)]:	
        line = line.strip()
        dep_old = line.split()[0]        
        dep_ext = os.path.splitext(dep_old)[1]
        if -1 < dep_old.find(old):
            dep_new = dep_old.replace(old, new)            
            if -1 < dep_old.find(name) and dep_ext == ext:
                id_changed = 1
                # Changing name of dylib.
                proc = subprocess.Popen('install_name_tool -id ' + dep_new + ' ' + fn, shell=True)
                sts = os.waitpid(proc.pid, 0)[1]
            else:
                # Changing name of dependencies.
                proc = subprocess.Popen('install_name_tool -change ' + dep_old + ' ' + dep_new + ' ' + fn, shell=True)
                sts = os.waitpid(proc.pid, 0)[1]

    # Checking replacement succcess:
    pipe = subprocess.Popen('otool -L ' + fn, shell=True, stdout=subprocess.PIPE).stdout
    output = pipe.readlines()
    for line in output[1:len(output)]:
        line = line.strip()
        dep_old = line.split()[0]
        if -1 < dep_old.find(old):        
           print '  Warning: replacement failed:', fn, dep_old
    
    if not id_changed and ext == '.dylib':
        print '  Warning: library id not changed:', fn, name

def process_dir(args, dirname, filelist):
    for filename in filelist:
        parts = os.path.splitext(filename)
        is_gst_tool = 0
        if -1 < filename.find('gst-'):
            is_gst_tool = 1
        is_lc_tool = 0
        if -1 < filename.find('leechcraft') or -1 < filename.find('lc-'):
            is_lc_tool = 1
        ext = parts[1]
        if ext == '.dylib' or ext == '.so' or is_gst_tool or is_lc_tool:
            fn = os.path.join(dirname, filename)
            process_file(args[0], args[1], filename, fn)

def main():
    old_string = ''
    new_string = ''
    input_dir = ''
    recursive = 0
    
    next_is_value = 0
    l = len(sys.argv)
    if 1 < l:
        for i in range(1, l):
            if next_is_value:
                next_is_value = 0 
                continue
            arg = sys.argv[i]
            if arg == '--old':
                if i + 1 < l:
                    next_is_value = 1
                    old_string = sys.argv[i+1]
                else:
                    print_error('Missing argument value')
                    return
            elif arg == '--new':
                if i + 1 < l:
                    next_is_value = 1
                    new_string = sys.argv[i+1]
                else:
                    print_error('Missing argument value')
                    return
            elif arg == '--dir':
                if i + 1 < l:
                    next_is_value = 1
                    input_dir = sys.argv[i+1]
                else:
                    print_error('Missing argument value')
                    return
            elif arg == '-R':
                recursive = 1
            elif arg == '--help':
                print_help()
                return
            else:
                print_error('Unknown argument')
                return
    else:
        print_help()
        return

    if old_string == '':
        print_error('Old string parameter is empty')
        return

    if new_string == '':
        print_error('New string parameter is empty')
        return

    if input_dir == '':
        print_error('Input directory parameter is empty')

    print 'WORKING...'
    if recursive:
        os.path.walk(input_dir, process_dir, [old_string, new_string])
    else:
        filelist = os.listdir(input_dir)
        process_dir([old_string, new_string], input_dir, filelist)
    print 'DONE.'
main()
