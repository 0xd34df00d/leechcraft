# Find Wt includes and libraries

#
# Wt_INCLUDE_DIR
# Wt_LIBRARIES  - Release libraries
# Wt_DEBUG_LIBRARIES  - Debug libraries
# Wt_DEBUG_FOUND  - True if debug libraries found
# Wt_LIBRARY_FOUND - True if core Wt library found
# Wt_EXT_LIBRARY_FOUND - True if ExtJS Wt library found
# Wt_HTTP_LIBRARY_FOUND - True if HTTP Wt library found
# Wt_FCGI_LIBRARY_FOUND - True if FCGI Wt library found


#
# Copyright (c) 2007, Pau Garcia i Quiles, <pgquiles@elpauer.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Some modifications, copyright (c) 2008, Rudoy Georg,
# 0xd34df00d@gmail.com>

find_path( Wt_INCLUDE_DIR NAMES WObject.h PATHS ENV PATH PATH_SUFFIXES include wt Wt )

set( Wt_FIND_COMPONENTS Release Debug )

if( Wt_INCLUDE_DIR )
        find_library( Wt_LIBRARY NAMES wt PATHS PATH PATH_SUFFIXES lib lib-release lib_release )
        find_library( Wt_EXT_LIBRARY NAMES wtext PATHS PATH PATH_SUFFIXES lib lib-release lib_release )
        find_library( Wt_HTTP_LIBRARY NAMES wthttp PATHS PATH PATH_SUFFIXES lib lib-release lib_release )
        find_library( Wt_FCGI_LIBRARY NAMES wtfcgi PATHS PATH PATH_SUFFIXES lib lib-release lib_release )

        find_library( Wt_DEBUG_LIBRARY NAMES wtd PATHS PATH PATH_SUFFIXES lib libd lib-debug lib_debug )
        find_library( Wt_EXT_DEBUG_LIBRARY NAMES wtextd PATHS PATH PATH_SUFFIXES lib libd lib-debug lib_debug )
        find_library( Wt_HTTP_DEBUG_LIBRARY NAMES wthttpd PATHS PATH PATH_SUFFIXES lib libd lib-debug lib_debug )
        find_library( Wt_FCGI_DEBUG_LIBRARY NAMES wtfcgid PATHS PATH PATH_SUFFIXES lib libd lib-debug lib_debug )

        if( Wt_LIBRARY OR Wt_EXT_LIBRARY OR Wt_HTTP_LIBRARY OR Wt_FCGI_LIBRARY )
				set( Wt_FOUND TRUE )
				set( Wt_FIND_REQUIRED_Release TRUE )
				set( Wt_LIBRARIES ${Wt_LIBRARY} ${Wt_EXT_LIBRARY} ${Wt_HTTP_LIBRARY} ${Wt_FCGI_LIBRARY} )
        endif()

		iF (Wt_LIBRARY)
			set (Wt_LIBRARY_FOUND TRUE)
		endif ()
		iF (Wt_EXT_LIBRARY)
			set (Wt_EXT_LIBRARY_FOUND TRUE)
		endif ()
		iF (Wt_HTTP_LIBRARY)
			set (Wt_HTTP_LIBRARY_FOUND TRUE)
		endif ()
		iF (Wt_FCGI_LIBRARY)
			set (Wt_FCGI_LIBRARY_FOUND TRUE)
		endif ()

        if( Wt_DEBUG_LIBRARY AND Wt_EXT_DEBUG_LIBRARY AND Wt_HTTP_DEBUG_LIBRARY AND Wt_FCGI_DEBUG_LIBRARY )
                set( Wt_DEBUG_FOUND TRUE )
		set( Wt_FIND_REQUIRED_Debug TRUE )
                set( Wt_DEBUG_LIBRARIES ${Wt_DEBUG_LIBRARY} ${Wt_EXT_DEBUG_LIBRARY} ${Wt_HTTP_DEBUG_LIBRARY} ${Wt_FCGI_DEBUG_LIBRARY} )
        endif()

        if(Wt_FOUND)
                if (NOT Wt_FIND_QUIETLY)
                        message(STATUS "Found the Wt libraries at ${Wt_LIBRARIES}")
                        message(STATUS "Found the Wt headers at ${Wt_INCLUDE_DIR}")
                endif ()
        else()
                if(Wt_FIND_REQUIRED)
                        message(FATAL_ERROR "Could NOT find Wt")
                endif()
        endif()

        if(Wt_DEBUG_FOUND)
                if (NOT Wt_FIND_QUIETLY)
                        message(STATUS "Found the Wt debug libraries at ${Wt_DEBUG_LIBRARIES}")
                        message(STATUS "Found the Wt debug headers at ${Wt_INCLUDE_DIR}")
                endif ()
        else()
                if(Wt_FIND_REQUIRED_Debug)
                        message(FATAL_ERROR "Could NOT find Wt debug libraries")
                endif()
        endif()

endif()
