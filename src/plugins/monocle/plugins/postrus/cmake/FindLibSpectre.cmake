# - Try to find the libspectre PS library
# Once done this will define
#
#  LIBSPECTRE_FOUND - system has libspectre
#  LIBSPECTRE_INCLUDE_DIR - the libspectre include directory
#  LIBSPECTRE_LIBRARY - Link this to use libspectre
#

# Copyright (c) 2006-2007, Pino Toscano, <pino@kde.org>
# Copyright (c) 2008, Albert Astals Cid, <aacid@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(LIBSPECTRE_INCLUDE_DIR AND LIBSPECTRE_LIBRARY)

  # in cache already
  set(LIBSPECTRE_FOUND TRUE)

else()

if(NOT WIN32)
   # use pkg-config to get the directories and then use these values
   # in the find_path() and find_library() calls
   include(UsePkgConfig)

   PKGCONFIG(libspectre _SpectreIncDir _SpectreLinkDir _SpectreLinkFlags _SpectreCflags)

   if(_SpectreLinkFlags)
     # find again pkg-config, to query it about libspectre version
     find_program(PKGCONFIG_EXECUTABLE NAMES pkg-config PATHS /usr/bin/ /usr/local/bin )

     # query pkg-config asking for a libspectre >= LIBSPECTRE_MINIMUM_VERSION
     exec_program(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=${LIBSPECTRE_MINIMUM_VERSION} libspectre RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )
     if(_return_VALUE STREQUAL "0")
        set(LIBSPECTRE_FOUND TRUE)
     endif()
   endif()
else()
    # do not use pkg-config on windows
    find_library(_SpectreLinkFlags NAMES libspectre spectre PATHS ${CMAKE_LIBRARY_PATH} ${SPECTRE_DIR}/lib)

	find_path(LIBSPECTRE_INCLUDE_DIR libspectre/spectre.h HINTS ${SPECTRE_DIR}/include)

    set(LIBSPECTRE_FOUND TRUE)
endif()

if (LIBSPECTRE_FOUND)
  set(LIBSPECTRE_LIBRARY ${_SpectreLinkFlags})

  # the cflags for libspectre can contain more than one include path
  separate_arguments(_SpectreCflags)
  foreach(_includedir ${_SpectreCflags})
    string(REGEX REPLACE "-I(.+)" "\\1" _includedir "${_includedir}")
    set(LIBSPECTRE_INCLUDE_DIR ${LIBSPECTRE_INCLUDE_DIR} ${_includedir})
  endforeach()

endif ()

# ensure that they are cached
set(LIBSPECTRE_INCLUDE_DIR ${LIBSPECTRE_INCLUDE_DIR} CACHE INTERNAL "The libspectre include path")
set(LIBSPECTRE_LIBRARY ${LIBSPECTRE_LIBRARY} CACHE INTERNAL "The libspectre library")

endif()
