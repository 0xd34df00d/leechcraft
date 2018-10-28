# Find libtorrent-rasterbar library

#
# RBTorrent_INCLUDE_DIR
# RBTorrent_LIBRARY
# RBTorrent_FOUND

# Copyright (c) 2008 Voker57 (voker57@gmail.com)
# Added more introspection for WIN32 (c) 2011 DZhon (TheDZhon@gmail.com)

if (WIN32)
	if (NOT DEFINED RBTorrent_DIR)
		if (RBTorrent_FIND_REQUIRED)
			message(FATAL_ERROR "Please set RBTorrent_DIR variable")
		else ()
			message(STATUS "Please set RBTorrent_DIR variable for libtorrent support")
		endif ()
	endif ()
endif ()

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
		else ()
			find_library (RBTorrent_LIBRARY NAMES libtorrent.dll.a PATHS ${RBTorrent_DIR}/lib)
		endif ()
	else ()
		find_library (RBTorrent_LIBRARY NAMES torrent-rasterbar PATH ENV)
	endif ()
endif ()

if (RBTorrent_INCLUDE_DIR AND RBTorrent_LIBRARY)
	set (RBTorrent_FOUND 1)
endif ()

if (RBTorrent_FOUND)
	message (STATUS "Found the libtorrent-rasterbar libraries at ${RBTorrent_LIBRARY}")
	message (STATUS "Found the libtorrent-rasterbar headers at ${RBTorrent_INCLUDE_DIR}")
	if (WIN32)
		message(STATUS ${_WIN32_ADDITIONAL_MESS})
	endif ()
else ()
	if (RBTorrent_FIND_REQUIRED)
		message (FATAL_ERROR "Could NOT find required libtorrent-rasterbar library, aborting")
	else ()
		message (STATUS "Could NOT find libtorrent-rasterbar")
	endif ()
endif ()
