find_package (PkgConfig)
pkg_check_modules (PC_SODIUM QUIET sodium)

find_path (SODIUM_INCLUDE_DIRS
	NAMES sodium.h
	PATHS
		/usr/local/include
		/usr/include
	HINTS
		${PC_SODIUM_INCLUDE_DIRS}
	)

find_library (SODIUM_LIBRARIES
	NAMES sodium
	PATHS
		/usr/local/lib
		/usr/lib
	HINTS
		${PC_SODIUM_LIBRARY_DIRS}
	)

if (SODIUM_INCLUDE_DIRS AND SODIUM_LIBRARIES)
	message (STATUS "Found Sodium includes at: ${SODIUM_INCLUDE_DIRS}")
	message (STATUS "Found Sodium libraries at: ${SODIUM_LIBRARIES}")
else ()
	message (FATAL_ERROR "Could NOT find Sodium")
endif ()
