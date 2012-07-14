# cmake macro to test LIBMSN LIB

# Copyright (c) 2006, Alessandro Praduroux <pradu@pradu.it>
# Copyright (c) 2007, Urs Wolfer <uwolfer @ kde.org>
# Copyright (c) 2008, Matt Rogers <mattr@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF (LIBMSN_INCLUDE_DIR AND LIBMSN_LIBRARIES)
    # Already in cache, be silent
    SET(LIBMSN_FIND_QUIETLY TRUE)
ENDIF (LIBMSN_INCLUDE_DIR AND LIBMSN_LIBRARIES)

FIND_PATH(LIBMSN_INCLUDE_DIR msn/msn.h
    PATHS ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES msn )

FIND_LIBRARY(LIBMSN_LIBRARIES NAMES msn libmsn
    PATHS ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES lib)

if (LIBMSN_INCLUDE_DIR AND LIBMSN_LIBRARIES)
  SET(LIBMSN_FOUND TRUE)
endif(LIBMSN_INCLUDE_DIR AND LIBMSN_LIBRARIES)

IF (LIBMSN_FOUND)
  IF (NOT LIBMSN_FIND_QUIETLY)
    MESSAGE(STATUS "Found LibMSN: ${LIBMSN_LIBRARIES}")
  ENDIF (NOT LIBMSN_FIND_QUIETLY)
ELSE (LIBMSN_FOUND)
  IF (LIBMSN_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could NOT find an acceptable version of LibMSN (version 0.4 or later required)")
  ENDIF (LIBMSN_FIND_REQUIRED)
ENDIF (LIBMSN_FOUND)

MARK_AS_ADVANCED(LIBMSN_INCLUDE_DIR LIBMSN_LIBRARIES)
