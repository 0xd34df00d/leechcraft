# - Try to find libvlc 0.9
# Once done this will define
#
#  VLC_FOUND - system has libvlc
#  VLC_INCLUDE_DIRS - the livlc include directory
#  VLC_LIBRARIES - Link these to use libvlc
#
#  Copyright (C) 2008  Tanguy Krotoff <tkrotoff@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
#  Added introspection for WIN32 (c) 2011 DZhon (TheDZhon@gmail.com)

if (VLC_LIBRARIES AND VLC_INCLUDE_DIRS)
	# in cache already
	set (VLC_FOUND TRUE)
else ()

	find_path (VLC_INCLUDE_DIR
		NAMES
		vlc/libvlc.h
		PATHS
		/usr/include
		/usr/local/include
		/opt/local/include
		/sw/include
		${VLC_DIR}/sdk/include
	)

	if (NOT MSVC)
		find_library (VLC_LIBRARY
			NAMES
			vlc
			PATHS
			/usr/lib
			/usr/local/lib
			/opt/local/lib
			/sw/lib
		)
	else ()
		find_library (VLC_LIBRARY
			NAMES
			libvlc.lib
			PATHS
			${VLC_DIR}
		)
	endif ()

	set (VLC_INCLUDE_DIRS
		${VLC_INCLUDE_DIR}
	)
	set (VLC_LIBRARIES
		${VLC_LIBRARY}
	)

	if (VLC_INCLUDE_DIRS AND VLC_LIBRARIES)
		set (VLC_FOUND TRUE)
	endif ()

	if (VLC_FOUND)
		if (NOT VLC_FIND_QUIETLY)
			message (STATUS "Found libvlc: ${VLC_LIBRARIES}")
		endif ()
	else ()
		if (VLC_FIND_REQUIRED)
			message (FATAL_ERROR "Could not find libvlc")
		endif ()
	endif ()

	# show the VLC_INCLUDE_DIRS and VLC_LIBRARIES variables only in the advanced view
	mark_as_advanced (VLC_INCLUDE_DIRS VLC_LIBRARIES)

endif ()
