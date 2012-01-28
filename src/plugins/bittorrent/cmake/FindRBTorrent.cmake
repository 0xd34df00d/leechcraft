# Find libtorrent-rasterbar library

#
# RBTorrent_INCLUDE_DIR
# RBTorrent_LIBRARY
# RBTorrent_FOUND

# Copyright (c) 2008 Voker57 (voker57@gmail.com)
# Added more introspection for WIN32 (c) 2011 DZhon (TheDZhon@gmail.com)

IF (WIN32)
	IF (MSVC)
		#MSVS 2010
		IF (MSVC_VERSION LESS 1600)
			MESSAGE(FATAL_ERROR "We currently support only MSVC 2010 version")
		ENDIF (MSVC_VERSION LESS 1600)
	ENDIF (MSVC)
	IF (NOT DEFINED RBTorrent_DIR)
		IF (RBTorrent_FIND_REQUIRED)
			MESSAGE(FATAL_ERROR "Please set RBTorrent_DIR variable")
		ELSE (RBTorrent_FIND_REQUIRED)
			MESSAGE(STATUS "Please set RBTorrent_DIR variable for libtorrent support")
		ENDIF (BTorrent_FIND_REQUIRED)
	ENDIF (NOT DEFINED RBTorrent_DIR)
ENDIF (WIN32)

FIND_PATH (RBTorrent_INCLUDE_DIR 
	NAMES 
		libtorrent/torrent.hpp 
	PATHS
		${RBTorrent_DIR}/include
		ENV PATH
	)
	
IF (RBTorrent_INCLUDE_DIR)
	IF (WIN32)
		SET (PROBE_DIR_Debug 
			${RBTorrent_DIR}/bin/msvc-10.0/debug/boost-link-shared/boost-source/threading-multi/)
		SET (PROBE_DIR_Release 
			${RBTorrent_DIR}/bin/msvc-10.0/release/boost-link-shared/boost-source/threading-multi/)
	
		FIND_LIBRARY (RBTorrent_LIBRARY_DEBUG NAMES torrent.lib PATHS ${PROBE_DIR_Debug})
		FIND_LIBRARY (RBTorrent_LIBRARY_RELEASE NAMES torrent.lib PATHS ${PROBE_DIR_Release})
		
		win32_tune_libs_names (RBTorrent)
		SET (RBTorrent_LIBRARY ${RBTorrent_LIBRARIES})
	ELSE (WIN32)
		FIND_LIBRARY (RBTorrent_LIBRARY NAMES torrent-rasterbar PATH ENV)
	ENDIF (WIN32)
ENDIF (RBTorrent_INCLUDE_DIR)

IF (RBTorrent_INCLUDE_DIR AND RBTorrent_LIBRARY)
	SET (RBTorrent_FOUND 1)
ENDIF (RBTorrent_INCLUDE_DIR AND RBTorrent_LIBRARY)

IF (RBTorrent_FOUND)
	MESSAGE (STATUS "Found the libtorrent-rasterbar libraries at ${RBTorrent_LIBRARY}")
	MESSAGE (STATUS "Found the libtorrent-rasterbar headers at ${RBTorrent_INCLUDE_DIR}")
	IF (WIN32)
		MESSAGE(STATUS ${_WIN32_ADDITIONAL_MESS})
	ENDIF (WIN32)
ELSE (RBTorrent_FOUND)
	IF (RBTorrent_FIND_REQUIRED)
		MESSAGE (FATAL_ERROR "Could NOT find required libtorrent-rasterbar library, aborting")
	ELSE (RBTorrent_FIND_REQUIRED)
		MESSAGE (STATUS "Could NOT find libtorrent-rasterbar")
	ENDIF (RBTorrent_FIND_REQUIRED)
ENDIF (RBTorrent_FOUND)
