# Find LibGuess - library for charset detection
#
# This module defines
#  LibGuess_FOUND - whether the libguess library was found
#  LibGuess_LIBRARIES - the libguess library
#  LibGuess_INCLUDE_DIR - the include path of the libguess library
#

if (LibGuess_INCLUDE_DIR AND LibGuess_LIBRARIES)
	# Already in cache
	set (LibGuess_FOUND TRUE)
else ()
	if (NOT WIN32)
		find_library (LibGuess_LIBRARY
			NAMES
				guess
			PATHS
				ENV
		)
	else ()
		if (NOT DEFINED LibGuess_DIR)
			if (LibGuess_FIND_REQUIRED)
				message(FATAL_ERROR "Please set LibGuess_DIR variable")
			else ()
				message(STATUS "Please set LibGuess_DIR variable for DLNiwe")
			endif ()
		endif ()

		if (MINGW)
			find_library (LibGuess_LIBRARIES NAMES libguess.a PATHS ${LibGuess_DIR}/lib)
		endif ()
	endif ()

	find_path (LibGuess_INCLUDE_DIR
		NAMES
			libguess/libguess.h
		PATHS
			${LibGuess_DIR}/include
			${INCLUDE_INSTALL_DIR}
	)

	if (LibGuess_INCLUDE_DIR AND LibGuess_LIBRARY)
		set (LibGuess_FOUND 1)
	endif ()

	if (LibGuess_FOUND)
		set (LibGuess_LIBRARIES ${LibGuess_LIBRARY})
		message (STATUS "Found the LibGuess libraries at ${LibGuess_LIBRARIES}")
		message (STATUS "Found the LibGuess headers at ${LibGuess_INCLUDE_DIR}")
		if (WIN32)
			message (STATUS ${_WIN32_ADDITIONAL_MESS})
		endif ()
	else ()
		if (LibGuess_FIND_REQUIRED)
			message (FATAL_ERROR "Could NOT find required LibGuess library, aborting")
		else ()
			message (STATUS "Could NOT find LibGuess")
		endif ()
	endif ()

endif ()

