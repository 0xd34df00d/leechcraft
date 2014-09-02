find_path (NACL_INCLUDE_DIRS
	NAMES nacl/crypto_box.h
	PATHS
		/usr/local/include
		/usr/include
	)

find_library (NACL_LIBRARIES
	NAMES nacl
	PATHS
		/usr/local/lib
		/usr/lib
	PATH_SUFFIXES
		nacl
	)

if (NACL_INCLUDE_DIRS AND NACL_LIBRARIES)
	message (STATUS "Found NaCl includes at: ${NACL_INCLUDE_DIRS}")
	message (STATUS "Found NaCl libraries at: ${NACL_LIBRARIES}")
else ()
	message (FATAL_ERROR "Could NOT find NaCl")
endif ()

