# - Try to find lastfm 0.3.3
# Once done this will define
#
#  LastFM_FOUND - system has liblastfm
#  LastFM_DIRS - the liblastfm include directory
#  LastFM_LIBRARIES - Link these to use liblastfm
#
#  Copyright (C) 2011  Minh Ngo <nlminhtl@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if (LastFM_LIBRARIES AND LastFM_INCLUDE_DIRS)
	# in cache already
	set (LastFM_FOUND TRUE)
else ()
	find_library (LastFM_LIBRARY
		NAMES
		lastfm-qt5
		lastfm5
		lastfm
		PATHS
		/usr/lib
		/usr/local/lib
		/usr/local/lib
		/opt/local/lib
		/sw/lib
		${LASTFM_DIR}/lib
	)
	find_path (LastFM_INCLUDE_DIR
		NAMES
		global.h
		PATH_SUFFIXES
		lastfm5
		lastfm
		PATHS
		/usr/include
		/usr/local/include
		/usr/local/include
		/opt/local/include
		/sw/include
		${LASTFM_DIR}/include
	)

	set (LastFM_INCLUDE_DIRS
		${LastFM_INCLUDE_DIR}
	)
	set (LastFM_LIBRARIES
		${LastFM_LIBRARY}
	)

	if (LastFM_INCLUDE_DIRS AND LastFM_LIBRARIES)
		set (LastFM_FOUND TRUE)
	endif ()

	if (LastFM_FOUND)
		if (NOT LastFM_FIND_QUIETLY)
			message (STATUS "Found liblastfm: ${LastFM_LIBRARIES}")
		endif ()
	else ()
		if (LastFM_FIND_REQUIRED)
			message (FATAL_ERROR "Could not find liblastfm")
		endif ()
	endif ()

	# show the LastFM_INCLUDE_DIRS and LastFM_LIBRARIES variables only in the advanced view
	mark_as_advanced (LastFM_INCLUDE_DIRS LastFM_LIBRARIES)

endif ()
