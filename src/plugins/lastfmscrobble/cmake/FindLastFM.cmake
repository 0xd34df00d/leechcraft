# - Try to find lastfm
#
#  Copyright (C) 2011  Minh Ngo <nlminhtl@gmail.com>
#  Copyright (C) 2020  Georg Rudoy <0xd34df00d@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

foreach (QT_VER 5 6)
	if (NOT (LastFM${QT_VER}_LIBRARY OR LastFM${QT_VER}_INCLUDE_DIR))
		find_library (LastFM${QT_VER}_LIBRARY NAMES lastfm-qt${QT_VER} lastfm${QT_VER} lastfm)
		find_path (LastFM${QT_VER}_INCLUDE_DIR NAMES global.h PATH_SUFFIXES lastfm${QT_VER} lastfm)
		message (STATUS "Looking for LastFM (Qt ${QT_VER})… libraries: ${LastFM${QT_VER}_LIBRARY}; includes: ${LastFM${QT_VER}_INCLUDE_DIR}")
	endif ()

	if (LastFM${QT_VER}_LIBRARY AND LastFM${QT_VER}_INCLUDE_DIR)
		add_library (LastFM::LastFM${QT_VER} UNKNOWN IMPORTED)
		set_target_properties (LastFM::LastFM${QT_VER} PROPERTIES
			IMPORTED_LOCATION "${LastFM${QT_VER}_LIBRARY}"
			INTERFACE_INCLUDE_DIRECTORIES "${LastFM${QT_VER}_INCLUDE_DIR}")
	endif ()
endforeach ()
