# Find QJSON - JSON handling library for Qt
#
# This module defines
#  QJSON_FOUND - whether the qsjon library was found
#  QJSON_LIBRARIES - the qjson library
#  QJSON_INCLUDE_DIR - the include path of the qjson library
#
# Added more introspection for WIN32 (c) 2011 DZhon (TheDZhon@gmail.com)

if (QJSON_INCLUDE_DIR AND QJSON_LIBRARIES)
	# Already in cache
	set (QJSON_FOUND TRUE)
else ()
	if (NOT WIN32)
		find_library (QJSON_LIBRARIES
			NAMES
				qjson
			PATHS
				${QJSON_DIR}
				${LIB_INSTALL_DIR}
				${KDE4_LIB_DIR}
		)
	else ()
		if (NOT DEFINED QJSON_DIR)
			if (QJSON_FIND_REQUIRED)
				message(FATAL_ERROR "Please set QJSON_DIR variable")
			else ()
				message(STATUS "Please set QJSON_DIR variable for onlinebookmarks support")
			endif ()
		endif ()
	
		if (MSVC)
			set (QJSON_INCLUDE_WIN32 ${QJSON_DIR})		
	
			set (PROBE_DIR_Debug
				${QJSON_DIR}/build/lib/Debug)
			set (PROBE_DIR_Release
				${QJSON_DIR}/build/lib/MinSizeRel ${QJSON_DIR}/build/lib/Release)
	
			find_library (QJSON_LIBRARY_DEBUG NAMES qjson.lib PATHS ${PROBE_DIR_Debug})
			find_library (QJSON_LIBRARY_RELEASE NAMES qjson.lib PATHS ${PROBE_DIR_Release})
			win32_tune_libs_names (QJSON)
		else ()
			find_library (QJSON_LIBRARIES NAMES libqjson.dll.a PATHS ${QJSON_DIR}/build/lib)
		endif ()
	endif ()

if (NOT WIN32) # regression guard
	find_path (QJSON_INCLUDE_DIR
    NAMES
		qjson_export.h
	PATH_SUFFIXES
		qjson
    PATHS
		${QJSON_DIR}
		${INCLUDE_INSTALL_DIR}
		${KDE4_INCLUDE_DIR}
	)
else () #may be works for linux too
	find_path (QJSON_INCLUDE_DIR
    NAMES
		qjson/qjson_export.h
	PATH_SUFFIXES
		qjson
    PATHS
		${QJSON_DIR}
		${INCLUDE_INSTALL_DIR}
		${KDE4_INCLUDE_DIR}
		${QJSON_INCLUDE_WIN32}
	)
endif ()

#Win32 Strange beg
	
	if (QJSON_INCLUDE_DIR AND QJSON_LIBRARIES)
		set (QJSON_FOUND 1)
	endif ()

	if (QJSON_FOUND)
		message (STATUS "Found the QJSON libraries at ${QJSON_LIBRARIES}")
		message (STATUS "Found the QJSON headers at ${QJSON_INCLUDE_DIR}")
		if (WIN32)
			message (STATUS ${_WIN32_ADDITIONAL_MESS})
		endif ()
	else ()
		if (QJSON_FIND_REQUIRED)
			message (FATAL_ERROR "Could NOT find required QJSON library, aborting")
		else ()
			message (STATUS "Could NOT find QJSON")
		endif ()
	endif ()

endif ()
