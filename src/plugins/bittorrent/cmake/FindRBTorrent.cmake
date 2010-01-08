# Find libtorrent-rasterbar library

#
# RBTorrent_INCLUDE_DIR
# RBTorrent_LIBRARY
# RBTorrent_FOUND

# Copyright (c) 2008 Voker57 (voker57@gmail.com)

#IF(NOT DEFINED RBTorrent_INCLUDE_DIR)
	FIND_PATH(RBTorrent_INCLUDE_DIR NAMES libtorrent/torrent.hpp PATH ENV)
#ENDIF()
#IF(NOT DEFINED RBTorrent_LIBRARY)
	FIND_LIBRARY(RBTorrent_LIBRARY NAMES torrent-rasterbar)
#ENDIF()

IF(RBTorrent_INCLUDE_DIR AND RBTorrent_LIBRARY)
	SET(RBTorrent_FOUND 1)
ENDIF()

IF(RBTorrent_FOUND)
	MESSAGE(STATUS "Found the libtorrent-rasterbar libraries at ${RBTorrent_LIBRARY}")
	MESSAGE(STATUS "Found the libtorrent-rasterbar headers at ${RBTorrent_INCLUDE_DIR}")
ELSE()
	IF(RBTorrent_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could NOT find required libtorrent-rasterbar library, aborting")
	ELSE()
		MESSAGE(STATUS "Could NOT find libtorrent-rasterbar")
	ENDIF()
ENDIF()
