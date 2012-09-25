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

IF (SPEEX_LIBRARIES AND SPEEX_INCLUDE_DIRS)
	# in cache already
	SET (SPEEX_FOUND TRUE)
ELSE (SPEEX_LIBRARIES AND SPEEX_INCLUDE_DIRS)
	# use pkg-config to get the directories and then use these values
	# in the FIND_PATH() and FIND_LIBRARY() calls
	#include(UsePkgConfig)

	#FIXME pkgconfig does not work: return a carriage return that makes compilation failed
	#pkgconfig(speex _SpeexIncDir _SpeexLinkDir _SpeexLinkFlags _SpeexCflags)

	#set(SPEEX_DEFINITIONS ${_SpeexCflags})
	SET (SPEEX_DEFINITIONS "")
  
	IF (WIN32)
		IF (NOT DEFINED SPEEX_DIR)
			IF (SPEEX_FIND_REQUIRED)
				MESSAGE (FATAL_ERROR "Please set SPEEX_DIR variable")
			ELSE (SPEEX_FIND_REQUIRED)
				MESSAGE (STATUS "Please set SPEEX_DIR variable for speex support")
			ENDIF (SPEEX_FIND_REQUIRED)
		ENDIF (NOT DEFINED SPEEX_DIR)
		SET (SPEEX_INCLUDE_WIN32 ${SPEEX_DIR}/include)
		SET (SPEEX_LIB_WIN32 ${SPEEX_DIR}/lib)
	ENDIF (WIN32)  
  
	FIND_PATH (SPEEX_INCLUDE_DIR
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

	FIND_LIBRARY (SPEEX_LIBRARY
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

	IF (SPEEX_LIBRARY)
		SET (SPEEX_FOUND TRUE)
	ENDIF (SPEEX_LIBRARY)

	SET (SPEEX_INCLUDE_DIRS
		${SPEEX_INCLUDE_DIR}
	)

	IF (SPEEX_FOUND)
		SET (SPEEX_LIBRARIES
			${SPEEX_LIBRARIES}
			${SPEEX_LIBRARY}
		)
	ENDIF (SPEEX_FOUND)

	IF (SPEEX_INCLUDE_DIRS AND SPEEX_LIBRARIES)
		SET (SPEEX_FOUND TRUE)
	ENDIF (SPEEX_INCLUDE_DIRS AND SPEEX_LIBRARIES)

	IF (SPEEX_FOUND)
		IF (NOT Speex_FIND_QUIETLY)
			MESSAGE (STATUS "Found Speex: ${SPEEX_LIBRARIES}")
		ENDIF (NOT Speex_FIND_QUIETLY)
		ELSE (SPEEX_FOUND)
    IF (Speex_FIND_REQUIRED)
		MESSAGE (FATAL_ERROR "Could not find Speex")
    ENDIF (Speex_FIND_REQUIRED)
	ENDIF (SPEEX_FOUND)

	# show the SPEEX_INCLUDE_DIRS and SPEEX_LIBRARIES variables only in the advanced view
	MARK_AS_ADVANCED (SPEEX_INCLUDE_DIRS SPEEX_LIBRARIES)

ENDIF (SPEEX_LIBRARIES AND SPEEX_INCLUDE_DIRS)
