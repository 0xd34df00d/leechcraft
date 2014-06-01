# - Try to find the Qt4 binding of the Poppler library
# Once done this will define
#
#  POPPLER_QT4_FOUND - system has poppler-qt4
#  POPPLER_QT4_INCLUDE_DIR - the poppler-qt4 include directory
#  POPPLER_QT4_LIBRARIES - Link these to use poppler-qt4
#  POPPLER_QT4_DEFINITIONS - Compiler switches required for using poppler-qt4
#

# use pkg-config to get the directories and then use these values
# in the find_path() and find_library() calls

# Copyright (c) 2006, Wilfried Huss, <wilfried.huss@gmx.at>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (UNIX)
	find_package(PkgConfig)
	pkg_check_modules(PC_POPPLERQT4 QUIET poppler-qt4)

	set(POPPLER_QT4_DEFINITIONS ${PC_POPPLERQT4_CFLAGS_OTHER})
endif ()

find_path (POPPLER_CPP_INCLUDE_DIR
	NAMES poppler-version.h
	PATHS ${POPPLER_DIR}/include
	PATH_SUFFIXES poppler/cpp cpp/src)

find_path (POPPLER_QT4_INCLUDE_DIR
	NAMES poppler-qt4.h
	PATHS ${PC_POPPLERQT4_INCLUDEDIR}
		${POPPLER_DIR}/include
 	PATH_SUFFIXES poppler/qt4 poppler qt4/src)

find_library(POPPLER_QT4_LIBRARY
	NAMES poppler-qt4 libpoppler-qt4.dll.a
	PATHS ${PC_POPPLERQT4_LIBDIR}
		${POPPLER_DIR}/lib
  	PATH_SUFFIXES build/qt4/src
)

set(POPPLER_QT4_LIBRARIES ${POPPLER_QT4_LIBRARY})

if (POPPLER_QT4_INCLUDE_DIR AND POPPLER_CPP_INCLUDE_DIR AND POPPLER_QT4_LIBRARIES)
  set(POPPLER_QT4_FOUND TRUE)
else ()
  set(POPPLER_QT4_FOUND FALSE)
endif ()

if (POPPLER_QT4_FOUND)
  if (NOT PopplerQt4_FIND_QUIETLY)
    message(STATUS "Found poppler-qt4: library: ${POPPLER_QT4_LIBRARIES}, include path: ${POPPLER_QT4_INCLUDE_DIR}; cpp include path: ${POPPLER_CPP_INCLUDE_DIR}")
  endif ()
else ()
  if (PopplerQt4_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find poppler-qt4, make sure both poppler-qt4 and poppler-cpp are installed")
  endif ()
endif ()

mark_as_advanced(POPPLER_QT4_INCLUDE_DIR POPPLER_QT4_LIBRARIES)
