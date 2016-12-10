# - Try to find HUNSPELL
# Once done this will define
#
#  HUNSPELL_FOUND - system has HUNSPELL
#  HUNSPELL_INCLUDE_DIR - the HUNSPELL include directory
#  HUNSPELL_LIBRARIES - The libraries needed to use HUNSPELL
#  HUNSPELL_DEFINITIONS - Compiler switches required for using HUNSPELL


if (HUNSPELL_INCLUDE_DIR AND HUNSPELL_LIBRARIES)
  # Already in cache, be silent
  set(HUNSPELL_FIND_QUIETLY TRUE)
endif ()

include(FindPackageHandleStandardArgs)

if (WIN32)
	if (MSVC)
		#MSVS 2010
		if (MSVC_VERSION LESS 1600)
			message(FATAL_ERROR "We currently support only MSVC 2010 or greater version")
		endif ()
	endif ()

	set (HUNSPELL_INCLUDE_WIN32 ${HUNSPELL_DIR}/src/)

	set (PROBE_DIR
		${HUNSPELL_DIR}/src/win_api/Release_dll/libhunspell)
endif ()

find_library (HUNSPELL_LIBRARIES NAMES hunspell-1.7 hunspell-1.6 hunspell-1.5 hunspell-1.4 hunspell-1.3 hunspell-1.2 libhunspell HINTS ${HUNSPELL_DIR} ${PROBE_DIR})
find_path (HUNSPELL_INCLUDE_DIR hunspell/hunspell.hxx HINTS ${HUNSPELL_DIR} ${HUNSPELL_INCLUDE_WIN32})

# handle the QUIETLY and REQUIRED arguments and set HUNSPELL_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS (HUNSPELL DEFAULT_MSG HUNSPELL_LIBRARIES HUNSPELL_INCLUDE_DIR)

if (HUNSPELL_FOUND)
  if (NOT HUNSPELL_FIND_QUIETLY AND HUNSPELLCONFIG_EXECUTABLE)
    message(STATUS "Hunspell found: ${HUNSPELL_LIBRARIES}")
  endif()
else()
  if(HUNSPELL_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find Hunspell")
  endif()
endif()

mark_as_advanced(HUNSPELL_INCLUDE_DIR HUNSPELL_LIBRARIES)
