# - Try to find libmagic 5.07
# Once done this will define
#
#  MAGIC_FOUND - system has libvlc
#  MAGIC_INCLUDE_DIRS - the livlc include directory
#  MAGIC_LIBRARIES - Link these to use libvlc
#
#  Copyright (C) 2011 Minh Ngo <nlminhtl@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if (MAGIC_LIBRARIES AND MAGIC_INCLUDE_DIRS)
	# in cache already
	set (MAGIC_FOUND TRUE)
else (MAGIC_LIBRARIES AND MAGIC_INCLUDE_DIRS)

	find_path (MAGIC_INCLUDE_DIR
		NAMES
		magic.h
		PATHS
		/usr/include
		/usr/local/include
		/opt/local/include
		/sw/include
	)

	find_library (MAGIC_LIBRARY
		NAMES
		magic
		PATHS
		/usr/lib
		/usr/local/lib
		/opt/local/lib
		/sw/lib
	)

	set (MAGIC_INCLUDE_DIRS
		${MAGIC_INCLUDE_DIR}
	)
	set (MAGIC_LIBRARIES
		${MAGIC_LIBRARY}
	)

	if (MAGIC_INCLUDE_DIRS AND MAGIC_LIBRARIES)
		set (MAGIC_FOUND TRUE)
	endif (MAGIC_INCLUDE_DIRS AND MAGIC_LIBRARIES)

	if (MAGIC_FOUND)
		if (NOT MAGIC_FIND_QUIETLY)
			message (STATUS "Found libmagic: ${MAGIC_LIBRARIES}")
		endif (NOT MAGIC_FIND_QUIETLY)
	else (MAGIC_FOUND)
		if (MAGIC_FIND_REQUIRED)
			message (FATAL_ERROR "Could not find libmagic")
		endif (MAGIC_FIND_REQUIRED)
	endif (MAGIC_FOUND)

	# show the MAGIC_INCLUDE_DIRS and MAGIC_LIBRARIES variables only in the advanced view
	mark_as_advanced (MAGIC_INCLUDE_DIRS MAGIC_LIBRARIES)

endif (MAGIC_LIBRARIES AND MAGIC_INCLUDE_DIRS)