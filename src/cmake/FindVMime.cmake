# - Try to find Libvmime
#
# Once done this will define
#
#  LIBVMIME_FOUND - system has Libvmime
#  LIBVMIME_INCLUDE_DIR - the Libvmime include directory
#  LIBVMIME_LIBRARIES - Link these to Libvmime
#  LIBVMIME_DEFINITIONS - Compiler switches required for using Libvmime

# Adapted from FindGnuTLS.cmake, which is:
#   Copyright 2009, Brad Hards, <bradh@kde.org>
#
# Changes are Copyright 2009, Michele Caini, <skypjack@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (LIBVMIME_INCLUDE_DIR AND LIBVMIME_LIBRARIES)
   # in cache already
   SET(Libvmime_FIND_QUIETLY TRUE)
ENDIF (LIBVMIME_INCLUDE_DIR AND LIBVMIME_LIBRARIES)

IF (NOT WIN32)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   find_package(PkgConfig)
   pkg_check_modules(PC_VMIME vmime)
   SET(LIBVMIME_DEFINITIONS ${PC_VMIME_CFLAGS_OTHER})
ENDIF (NOT WIN32)

FIND_PATH(LIBVMIME_INCLUDE_DIR vmime/vmime.hpp
    HINTS
    ${PC_VMIME_INCLUDEDIR}
    ${PC_VMIME_INCLUDE_DIRS}
    PATH_SUFFIXES vmime
  )

FIND_LIBRARY(LIBVMIME_LIBRARIES NAMES vmime libvmime
    HINTS
    ${PC_VMIME_LIBDIR}
    ${PC_VMIME_LIBRARY_DIRS}
  )

INCLUDE(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LIBVMIME_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBVMIME DEFAULT_MSG LIBVMIME_LIBRARIES LIBVMIME_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBVMIME_INCLUDE_DIR LIBVMIME_LIBRARIES)