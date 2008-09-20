# Find libtorrent-rasterbar library

#
# RBTorrent_INCLUDE_DIR
# RBTorrent_LIBRARY
# RBTorrent_FOUND

# Copyright (c) 2008 Voker57 (voker57@gmail.com)
IF(NOT DEFINED RBTorrent_INCLUDE_DIR AND NOT DEFINED RBTorrent_LIBRARY)

FIND_PATH(RBTorrent_INCLUDE_DIR NAMES libtorrent/torrent.hpp PATH ENV)
IF(RBTorrent_INCLUDE_DIR)
	FIND_LIBRARY(RBTorrent_LIBRARY NAMES torrent-rasterbar)
	IF(RBTorrent_LIBRARY)
		SET(RBTorrent_FOUND 1)
	ENDIF(RBTorrent_LIBRARY)
ENDIF(RBTorrent_INCLUDE_DIR)

IF(RBTorrent_FOUND)
	MESSAGE(STATUS "Found the libtorrent-rasterbar libraries at ${RBTorrent_LIBRARY}")
	MESSAGE(STATUS "Found the libtorrent-rasterbar headers at ${RBTorrent_INCLUDE_DIR}")
ELSE(RBTorrent_FOUND)
	IF(RBTorrent_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could NOT find required libtorrent-rasterbar library, aborting")
	ELSE(RBTorrent_FIND_REQUIRED)
		MESSAGE(STATUS "Could NOT find libtorrent-rasterbar")
	ENDIF(RBTorrent_FIND_REQUIRED)
ENDIF(RBTorrent_FOUND)

ENDIF(NOT DEFINED RBTorrent_INCLUDE_DIR AND NOT DEFINED RBTorrent_LIBRARY)