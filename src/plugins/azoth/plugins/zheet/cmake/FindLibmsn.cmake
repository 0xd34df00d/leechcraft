# cmake macro to test LIBMSN LIB

# Copyright (c) 2006, Alessandro Praduroux <pradu@pradu.it>
# Copyright (c) 2007, Urs Wolfer <uwolfer @ kde.org>
# Copyright (c) 2008, Matt Rogers <mattr@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (LIBMSN_INCLUDE_DIR AND LIBMSN_LIBRARIES)
    # Already in cache, be silent
    set(LIBMSN_FIND_QUIETLY TRUE)
endif ()

find_path(LIBMSN_INCLUDE_DIR msn/msn.h
    PATHS ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES msn )

find_library(LIBMSN_LIBRARIES NAMES msn libmsn
    PATHS ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES lib)

if (LIBMSN_INCLUDE_DIR AND LIBMSN_LIBRARIES)
  set(LIBMSN_FOUND TRUE)
endif()

if (LIBMSN_FOUND)
  if (NOT LIBMSN_FIND_QUIETLY)
    message(STATUS "Found LibMSN: ${LIBMSN_LIBRARIES}")
  endif ()
else ()
  if (LIBMSN_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find an acceptable version of LibMSN (version 0.4 or later required)")
  endif ()
endif ()

mark_as_advanced(LIBMSN_INCLUDE_DIR LIBMSN_LIBRARIES)
