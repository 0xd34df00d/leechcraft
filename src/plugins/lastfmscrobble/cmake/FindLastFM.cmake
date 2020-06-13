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

if (LastFM_LIBRARY AND LastFM_INCLUDE_DIR)
	set (LastFM_FOUND TRUE)
else ()
	find_library (LastFM_LIBRARY NAMES lastfm-qt5 lastfm5 lastfm)
	find_path (LastFM_INCLUDE_DIR NAMES global.h PATH_SUFFIXES lastfm5 lastfm)

	if (LastFM_INCLUDE_DIR AND LastFM_LIBRARY)
		set (LastFM_FOUND TRUE)
		message (STATUS "Found lastfm libraries at ${LastFM_LIBRARY}")
		message (STATUS "Found lastfm headers at ${LastFM_INCLUDE_DIR}")
	elseif (LastFM_FIND_REQUIRED)
		message (FATAL_ERROR "Could not find liblastfm")
	endif ()
endif ()

set (LastFM_INCLUDE_DIRS ${LastFM_INCLUDE_DIR})
set (LastFM_LIBRARIES ${LastFM_LIBRARY})
