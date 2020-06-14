# - Try to find lastfm
#
#  Copyright (C) 2011  Minh Ngo <nlminhtl@gmail.com>
#  Copyright (C) 2020  Georg Rudoy <0xd34df00d@gmail.com>
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
		mark_as_advanced(LastFM_LIBRARY LastFM_INCLUDE_DIR)
	endif ()
endif ()

if (LastFM_FOUND)
	add_library (LastFM::LastFM UNKNOWN IMPORTED)
	set_target_properties (LastFM::LastFM PROPERTIES
		IMPORTED_LOCATION "${LastFM_LIBRARY}"
		INTERFACE_INCLUDE_DIRECTORIES "${LastFM_INCLUDE_DIR}")
endif ()
