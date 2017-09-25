# - Try to find Speex
# Once done this will define
#
#  SPEEX_FOUND - system has Speex
#  SPEEX_INCLUDE_DIRS - the Speex include directory
#  SPEEX_LIBRARIES - Link these to use Speex
#  SPEEX_DEFINITIONS - Compiler switches required for using Speex
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
#  Added more introspection for WIN32 (c) 2011 DZhon (TheDZhon@gmail.com)

if (SPEEX_LIBRARIES AND SPEEX_INCLUDE_DIRS)
	# in cache already
	set (SPEEX_FOUND TRUE)
else ()
	# use pkg-config to get the directories and then use these values
	# in the find_path() and find_library() calls
	#include(UsePkgConfig)

	#set(SPEEX_DEFINITIONS ${_SpeexCflags})
	set (SPEEX_DEFINITIONS "")
  
	if (WIN32)
		if (NOT DEFINED SPEEX_DIR)
			if (SPEEX_FIND_REQUIRED)
				message (FATAL_ERROR "Please set SPEEX_DIR variable")
			else ()
				message (STATUS "Please set SPEEX_DIR variable for speex support")
			endif ()
		endif ()
		set (SPEEX_INCLUDE_WIN32 ${SPEEX_DIR}/include)
		set (SPEEX_LIB_WIN32 ${SPEEX_DIR}/lib)
	endif ()  
  
	find_path (SPEEX_INCLUDE_DIR
		NAMES
			speex/speex.h
			speex.h
		PATHS
			${_SpeexIncDir}
			/usr/include
			/usr/local/include
			/opt/local/include
			/sw/include
			${SPEEX_INCLUDE_WIN32}
	)

	find_library (SPEEX_LIBRARY
		NAMES
			speex
			Speex
		libspeex.lib
		PATHS
			${_SpeexLinkDir}
			/usr/lib
			/usr/local/lib
			/opt/local/lib
			/sw/lib
			${SPEEX_LIB_WIN32}
	)

	if (SPEEX_LIBRARY)
		set (SPEEX_FOUND TRUE)
	endif ()

	set (SPEEX_INCLUDE_DIRS
		${SPEEX_INCLUDE_DIR}
	)

	if (SPEEX_FOUND)
		set (SPEEX_LIBRARIES
			${SPEEX_LIBRARIES}
			${SPEEX_LIBRARY}
		)
	endif ()

	if (SPEEX_INCLUDE_DIRS AND SPEEX_LIBRARIES)
		set (SPEEX_FOUND TRUE)
	endif ()

	if (SPEEX_FOUND)
		if (NOT Speex_FIND_QUIETLY)
			message (STATUS "Found Speex: ${SPEEX_LIBRARIES}")
		endif ()
		else ()
    if (Speex_FIND_REQUIRED)
		message (FATAL_ERROR "Could not find Speex")
    endif ()
	endif ()

	# show the SPEEX_INCLUDE_DIRS and SPEEX_LIBRARIES variables only in the advanced view
	mark_as_advanced (SPEEX_INCLUDE_DIRS SPEEX_LIBRARIES)

endif ()
