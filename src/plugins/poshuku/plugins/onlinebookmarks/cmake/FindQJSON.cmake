# Find QJSON - JSON handling library for Qt
#
# This module defines
#  QJSON_FOUND - whether the qsjon library was found
#  QJSON_LIBRARIES - the qjson library
#  QJSON_INCLUDE_DIR - the include path of the qjson library
#
# Added more introspection for WIN32 (c) 2011 DZhon (TheDZhon@gmail.com)

IF (QJSON_INCLUDE_DIR AND QJSON_LIBRARIES)
	# Already in cache
	SET (QJSON_FOUND TRUE)
ELSE (QJSON_INCLUDE_DIR AND QJSON_LIBRARIES)
	IF (NOT WIN32)
    # use pkg-config to get the values of QJSON_INCLUDE_DIRS
    # and QJSON_LIBRARY_DIRS to add as hints to the find commands.
    #include (FindPkgConfig)
    #pkg_check_modules (QJSON REQUIRED QJson>=0.5)
	FIND_LIBRARY (QJSON_LIBRARIES
		NAMES
			qjson
		PATHS
			${QJSON_DIR}
			${LIB_INSTALL_DIR}
			${KDE4_LIB_DIR}
	)

	ELSE (NOT WIN32) 
	
		IF (NOT DEFINED QJSON_DIR)
			IF (QJSON_FIND_REQUIRED)
				MESSAGE(FATAL_ERROR "Please set QJSON_DIR variable")
			ELSE (QJSON_FIND_REQUIRED)
				MESSAGE(STATUS "Please set QJSON_DIR variable for onlinebookmarks support")
			ENDIF (QJSON_FIND_REQUIRED)
		ENDIF (NOT DEFINED QJSON_DIR)
  
		SET (QJSON_INCLUDE_WIN32 ${QJSON_DIR}/qjson)
	
		SET (PROBE_DIR_Debug 
			${QJSON_DIR}/build/lib/Debug)
		SET (PROBE_DIR_Release 
			${QJSON_DIR}/build/lib/MinSizeRel ${QJSON_DIR}/build/lib/Release)
			
		FIND_LIBRARY (QJSON_LIBRARY_Debug NAMES qjson.lib PATHS ${PROBE_DIR_Debug})
		FIND_LIBRARY (QJSON_LIBRARY_Release NAMES qjson.lib PATHS ${PROBE_DIR_Release})
	
		IF (PROBE_DIR_Debug AND PROBE_DIR_Release)
			IF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPES)
				SET (QJSON_LIBRARIES optimized ${QJSON_LIBRARY_Release} debug ${QJSON_LIBRARY_Debug})
				SET (_WIN32_ADDITIONAL_MESS "Weee: both debug and release QJSON versions available")
			ELSE (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPES)
				SET (QJSON_LIBRARIES ${QJSON_LIBRARY_Release})
				SET (_WIN32_ADDITIONAL_MESS "Warning: Your generator doesn't support separate debug and release libs") 	
			ENDIF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPES)
		ENDIF (PROBE_DIR_Debug AND PROBE_DIR_Release)
		IF (PROBE_DIR_Debug AND NOT PROBE_DIR_Release)
			SET (QJSON_LIBRARIES ${QJSON_LIBRARY_Debug})
			SET (_WIN32_ADDITIONAL_MESS "Warning: only debug QJSON library available")
		ENDIF (PROBE_DIR_Debug AND NOT PROBE_DIR_Release)
		IF (NOT PROBE_DIR_Debug AND PROBE_DIR_Release)
			SET (QJSON_LIBRARIES ${QJSON_LIBRARY_Release})
			SET (_WIN32_ADDITIONAL_MESS "Warning: only release QJSON library available")
		ENDIF (NOT PROBE_DIR_Debug AND PROBE_DIR_Release)
		
	ENDIF (NOT WIN32)

	FIND_PATH (QJSON_INCLUDE_DIR
    NAMES
		json_scanner.h
		PATH_SUFFIXES
		qjson
    PATHS
		${QJSON_DIR}
		${INCLUDE_INSTALL_DIR}
		${KDE4_INCLUDE_DIR}
		${QJSON_INCLUDE_WIN32}
	)
  
	IF (QJSON_INCLUDE_DIR AND QJSON_LIBRARIES)
		SET (QJSON_FOUND 1)
	ENDIF (QJSON_INCLUDE_DIR AND QJSON_LIBRARIES)
  
	IF (QJSON_FOUND)
		MESSAGE (STATUS "Found the QJSON libraries at ${QJSON_LIBRARIES}")
		MESSAGE (STATUS "Found the QJSON headers at ${QJSON_INCLUDE_DIR}")
		IF (WIN32)
			MESSAGE (STATUS ${_WIN32_ADDITIONAL_MESS})
		ENDIF (WIN32)
	ELSE (QJSON_FOUND)
		IF (QJSON_FIND_REQUIRED)
			MESSAGE (FATAL_ERROR "Could NOT find required QJSON library, aborting")
		ELSE (QJSON_FIND_REQUIRED)
			MESSAGE (STATUS "Could NOT find QJSON")
		ENDIF (QJSON_FIND_REQUIRED)
	ENDIF (QJSON_FOUND)
	
ENDIF (QJSON_INCLUDE_DIR AND QJSON_LIBRARIES)
