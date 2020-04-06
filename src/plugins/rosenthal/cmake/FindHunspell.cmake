# - Try to find Hunspell
# Once done this will define
#
#  Hunspell_FOUND - system has Hunspell
#  Hunspell_INCLUDE_DIR - the Hunspell include directory
#  Hunspell_LIBRARIES - The libraries needed to use Hunspell
#  Hunspell_DEFINITIONS - Compiler switches required for using Hunspell


if (Hunspell_INCLUDE_DIR AND Hunspell_LIBRARIES)
  # Already in cache, be silent
  set(Hunspell_FIND_QUIETLY TRUE)
endif ()

include(FindPackageHandleStandardArgs)

if (WIN32)
	if (MSVC)
		#MSVS 2010
		if (MSVC_VERSION LESS 1600)
			message(FATAL_ERROR "We currently support only MSVC 2010 or greater version")
		endif ()
	endif ()

	set (Hunspell_INCLUDE_WIN32 ${Hunspell_DIR}/src/)

	set (PROBE_DIR
		${Hunspell_DIR}/src/win_api/Release_dll/libhunspell)
endif ()

find_library (Hunspell_LIBRARIES NAMES hunspell-1.7 hunspell-1.6 hunspell-1.5 hunspell-1.4 hunspell-1.3 hunspell-1.2 libhunspell HINTS ${Hunspell_DIR} ${PROBE_DIR})
find_path (Hunspell_INCLUDE_DIR hunspell/hunspell.hxx HINTS ${Hunspell_DIR} ${Hunspell_INCLUDE_WIN32})

# handle the QUIETLY and REQUIRED arguments and set Hunspell_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS (Hunspell DEFAULT_MSG Hunspell_LIBRARIES Hunspell_INCLUDE_DIR)

if (Hunspell_FOUND)
  if (NOT Hunspell_FIND_QUIETLY AND HunspellCONFIG_EXECUTABLE)
    message(STATUS "Hunspell found: ${Hunspell_LIBRARIES}")
  endif()
else()
  if(Hunspell_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find Hunspell")
  endif()
endif()

mark_as_advanced(Hunspell_INCLUDE_DIR Hunspell_LIBRARIES)
