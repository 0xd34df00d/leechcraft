# Find Qross library

#
# QROSS_INCLUDE_DIR
# QROSS_LIBRARIES
# QROSS_FOUND

# Copyright (c) 2011 Georg Rudoy <0xd34df00d@gmail.com>

if(QROSS_LIBRARIES AND QROSS_INCLUDE_DIR)
	set(QROSS_FOUND 1)
else()
	find_path(QROSS_INCLUDE_DIR qross/core/manager.h PATH ENV)
	find_library(QROSS_LIBRARIES NAMES qrosscore-qt5)
	if(QROSS_LIBRARIES AND QROSS_INCLUDE_DIR)
		set(QROSS_FOUND 1)
	endif()
endif()

if(QROSS_FOUND)
	message(STATUS "Found Qross libraries at ${QROSS_LIBRARIES}")
	message(STATUS "Found Qross headers at ${QROSS_INCLUDE_DIR}")
else()
	if(QROSS_FIND_REQUIRED)
		message(FATAL_ERROR "Could NOT find required Qross library, aborting")
	else()
		message(STATUS "Could NOT find Qross")
	endif()
endif()
