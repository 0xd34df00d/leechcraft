find_path (TOX_INCLUDE_DIRS
	NAMES tox/tox.h
	PATHS
		/usr/local/include
		/usr/include
	)

find_library (TOXCORE_LIBRARY
	NAMES toxcore
	PATHS
		/usr/local/lib
		/usr/lib
	)
find_library (TOXAV_LIBRARY
	NAMES toxav
	PATHS
		/usr/local/lib
		/usr/lib
	)

if (TOX_INCLUDE_DIRS AND TOXCORE_LIBRARY AND TOXAV_LIBRARY)
	message (STATUS "Found Tox includes at: ${TOX_INCLUDE_DIRS}")
	message (STATUS "Found Tox core library at: ${TOXCORE_LIBRARY}")
	message (STATUS "Found Tox AV library at: ${TOXAV_LIBRARY}")
else ()
	message (FATAL_ERROR "Could NOT find Tox")
endif ()
