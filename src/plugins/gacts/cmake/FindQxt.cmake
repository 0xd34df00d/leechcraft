#############
## basic FindQxt.cmake
## This is an *EXTREMELY BASIC* cmake find/config file for
## those times you have a cmake project and wish to use
## libQxt.
##
## It should be noted that at the time of writing, that
## I (mschnee) have an extremely limited understanding of the
## way Find*.cmake files work, but I have attempted to
## emulate what FindQt4.cmake and a few others do.
##
##  To enable a specific component, set your QXT_USE_${modname}:
##  set(QXT_USE_QXTCORE TRUE)
##  set(QXT_USE_QXTGUI FALSE)
##  Currently available components:
##  QxtCore, QxtGui, QxtNetwork, QxtWeb, QxtSql
##  Auto-including directories are enabled with include_directories(), but
##  can be accessed if necessary via ${QXT_INCLUDE_DIRS}
##
## To add the libraries to your build, target_link_libraries(), such as...
##  target_link_libraries(YourTargetNameHere ${QXT_LIBRARIES})
## ...or..
##  target_link_libraries(YourTargetNameHere ${QT_LIBRARIES} ${QXT_LIBRARIES})
##############

###### setup
set(QXT_MODULES QxtGui QxtWeb QxtZeroConf QxtNetwork QxtSql QxtBerkeley QxtCore)
set(QXT_FOUND_MODULES)
foreach(mod ${QXT_MODULES})
    string(TOUPPER ${mod} U_MOD)
    set(QXT_${U_MOD}_INCLUDE_DIR NOTFOUND)
    set(QXT_${U_MOD}_LIB_DEBUG NOTFOUND)
    set(QXT_${U_MOD}_LIB_RELEASE NOTFOUND)
    set(QXT_FOUND_${U_MOD} FALSE)
endforeach()
set(QXT_QXTGUI_DEPENDSON QxtCore)
set(QXT_QXTWEB_DEPENDSON QxtCore QxtNetwork)
set(QXT_QXTZEROCONF_DEPENDSON QxtCore QxtNetwork)
set(QXT_QXTNETWORK_DEPENDSON QxtCore)
set(QXT_QXTQSQL_DEPENDSON QxtCore)
set(QXT_QXTBERKELEY_DEPENDSON QxtCore)

foreach(mod ${QXT_MODULES})
    string(TOUPPER ${mod} U_MOD)
    find_path(QXT_${U_MOD}_INCLUDE_DIR ${mod}
        PATH_SUFFIXES ${mod} include/${mod}
            qxt/include/${mod} include/qxt/${mod}
            Qxt/include/${mod} include/Qxt/${mod}
        PATHS
        ~/Library/Frameworks/
        /Library/Frameworks/
        /sw/
        /usr/local/
        /usr
        /opt/local/
        /opt/csw
        /opt
        "C:\\"
        "C:\\Program Files\\"
        "C:\\Program Files(x86)\\"
        NO_DEFAULT_PATH
    )
    find_library(QXT_${U_MOD}_LIB_RELEASE NAME ${mod}
        PATH_SUFFIXES Qxt/lib64 Qxt/lib lib64 lib lib/${CMAKE_LIBRARY_ARCHITECTURE}
        PATHS
        /sw
        /usr/local
        /usr
        /opt/local
        /opt/csw
        /opt
        "C:\\"
        "C:\\Program Files"
        "C:\\Program Files(x86)"
        NO_DEFAULT_PATH
    )
    find_library(QXT_${U_MOD}_LIB_DEBUG NAME ${mod}d
        PATH_SUFFIXES Qxt/lib64 Qxt/lib lib64 lib lib/${CMAKE_LIBRARY_ARCHITECTURE}
        PATHS
        /sw
        /usr/local
        /usr
        /opt/local
        /opt/csw
        /opt
        "C:\\"
        "C:\\Program Files"
        "C:\\Program Files(x86)"
        NO_DEFAULT_PATH
    )
    if (QXT_${U_MOD}_LIB_RELEASE)
        set(QXT_FOUND_MODULES "${QXT_FOUND_MODULES} ${mod}")
    endif ()

    if (QXT_${U_MOD}_LIB_DEBUG)
        set(QXT_FOUND_MODULES "${QXT_FOUND_MODULES} ${mod}")
    endif ()
endforeach()

foreach(mod ${QXT_MODULES})
    string(TOUPPER ${mod} U_MOD)
    if(QXT_${U_MOD}_INCLUDE_DIR AND QXT_${U_MOD}_LIB_RELEASE)
        set(QXT_FOUND_${U_MOD} TRUE)
    endif()
endforeach()


##### find and include
# To use a Qxt Library....
#   set(QXT_FIND_COMPONENTS QxtCore, QxtGui)
# ...and this will do the rest
if( QXT_FIND_COMPONENTS )
    foreach( component ${QXT_FIND_COMPONENTS} )
        string( TOUPPER ${component} _COMPONENT )
        set(QXT_USE_${_COMPONENT}_COMPONENT TRUE)
    endforeach()
endif()

set(QXT_LIBRARIES "")
set(QXT_INCLUDE_DIRS "")

# like FindQt4.cmake, in order of dependence
foreach( module ${QXT_MODULES} )
    string(TOUPPER ${module} U_MOD)
    if(QXT_USE_${U_MOD} OR QXT_DEPENDS_${U_MOD})
        if(QXT_FOUND_${U_MOD})
            string(REPLACE "QXT" "" qxt_module_def "${U_MOD}")
            add_definitions(-DQXT_${qxt_module_def}_LIB)
            set(QXT_INCLUDE_DIRS ${QXT_INCLUDE_DIRS} ${QXT_${U_MOD}_INCLUDE_DIR})
            include_directories(${QXT_${U_MOD}_INCLUDE_DIR})
            set(QXT_LIBRARIES ${QXT_LIBRARIES} ${QXT_${U_MOD}_LIB_RELEASE})
        else()
            message("Could not find Qxt Module ${module}")
            return()
        endif()
        foreach(dep QXT_${U_MOD}_DEPENDSON)
            set(QXT_DEPENDS_${dep} TRUE)
        endforeach()
    endif()
endforeach()
message(STATUS "Found Qxt Libraries:${QXT_FOUND_MODULES}")
