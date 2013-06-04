# Find libtorrent-rasterbar library

#
# RBTorrent_INCLUDE_DIR
# RBTorrent_LIBRARY
# RBTorrent_FOUND

# Copyright (c) 2008 Voker57 (voker57@gmail.com)
# Added more introspection for WIN32 (c) 2011 DZhon (TheDZhon@gmail.com)

if (WIN32)
	if (MSVC)
		#MSVS 2010
		if (MSVC_VERSION LESS 1600)
			message(FATAL_ERROR "We currently support only MSVC 2010 version")
		endif (MSVC_VERSION LESS 1600)
	endif (MSVC)
	if (NOT DEFINED RBTorrent_DIR)
		if (RBTorrent_FIND_REQUIRED)
			message(FATAL_ERROR "Please set RBTorrent_DIR variable")
		else (RBTorrent_FIND_REQUIRED)
			message(STATUS "Please set RBTorrent_DIR variable for libtorrent support")
		endif (BTorrent_FIND_REQUIRED)
	endif (NOT DEFINED RBTorrent_DIR)
endif (WIN32)

find_path (RBTorrent_INCLUDE_DIR 
	NAMES 
		libtorrent/torrent.hpp 
	PATHS
		${RBTorrent_DIR}/include
		ENV PATH
	)
	
if (RBTorrent_INCLUDE_DIR)
	if (WIN32)
		if (MSVC)
			set (PROBE_DIR_Debug 
				${RBTorrent_DIR}/bin/msvc-10.0/debug/boost-link-shared/boost-source/threading-multi/)
			set (PROBE_DIR_Release 
				${RBTorrent_DIR}/bin/msvc-10.0/release/boost-link-shared/boost-source/threading-multi/)
		
			find_library (RBTorrent_LIBRARY_DEBUG NAMES torrent.lib PATHS ${PROBE_DIR_Debug})
			find_library (RBTorrent_LIBRARY_RELEASE NAMES torrent.lib PATHS ${PROBE_DIR_Release})
			
			win32_tune_libs_names (RBTorrent)
			set (RBTorrent_LIBRARY ${RBTorrent_LIBRARIES})
		else (MSVC)
			find_library (RBTorrent_LIBRARY NAMES libtorrent.dll.a PATHS ${RBTorrent_DIR}/bin/gcc-mingw-4.7.2/release/boost-link-shared/boost-source/iconv-on/threading-multi)
		endif (MSVC)
	else (WIN32)
		find_library (RBTorrent_LIBRARY NAMES torrent-rasterbar PATH ENV)
	endif (WIN32)
endif (RBTorrent_INCLUDE_DIR)

if (RBTorrent_INCLUDE_DIR AND RBTorrent_LIBRARY)
	set (RBTorrent_FOUND 1)
endif (RBTorrent_INCLUDE_DIR AND RBTorrent_LIBRARY)

if (RBTorrent_FOUND)
	message (STATUS "Found the libtorrent-rasterbar libraries at ${RBTorrent_LIBRARY}")
	message (STATUS "Found the libtorrent-rasterbar headers at ${RBTorrent_INCLUDE_DIR}")
	if (WIN32)
		message(STATUS ${_WIN32_ADDITIONAL_MESS})
	endif (WIN32)
else (RBTorrent_FOUND)
	if (RBTorrent_FIND_REQUIRED)
		message (FATAL_ERROR "Could NOT find required libtorrent-rasterbar library, aborting")
	else (RBTorrent_FIND_REQUIRED)
		message (STATUS "Could NOT find libtorrent-rasterbar")
	endif (RBTorrent_FIND_REQUIRED)
endif (RBTorrent_FOUND)
