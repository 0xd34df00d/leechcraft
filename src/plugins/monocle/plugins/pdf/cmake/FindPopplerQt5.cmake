# - Try to find the Qt4 binding of the Poppler library
# Once done this will define
#
#  POPPLER_QT5_FOUND - system has poppler-qt5
#  POPPLER_QT5_INCLUDE_DIR - the poppler-qt5 include directory
#  POPPLER_QT5_LIBRARIES - Link these to use poppler-qt5
#  POPPLER_QT5_DEFINITIONS - Compiler switches required for using poppler-qt5
#

# use pkg-config to get the directories and then use these values
# in the find_path() and find_library() calls

# Copyright (c) 2006, Wilfried Huss, <wilfried.huss@gmx.at>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (UNIX)
	find_package(PkgConfig)
	pkg_check_modules(PC_POPPLERQT5 QUIET poppler-qt5)

	set(POPPLER_QT5_DEFINITIONS ${PC_POPPLERQT5_CFLAGS_OTHER})
endif ()

find_path (POPPLER_CPP_INCLUDE_DIR
	NAMES poppler-version.h
	PATHS ${POPPLER_DIR}/include
	PATH_SUFFIXES poppler/cpp cpp/src)

find_path (POPPLER_QT5_INCLUDE_DIR
	NAMES poppler-qt5.h
	PATHS ${PC_POPPLERQT5_INCLUDEDIR}
		${POPPLER_DIR}/include
 	PATH_SUFFIXES poppler/qt5 poppler qt5/src)

find_library(POPPLER_QT5_LIBRARY
	NAMES poppler-qt5 libpoppler-qt5.dll.a
	PATHS ${PC_POPPLERQT5_LIBDIR}
		${POPPLER_DIR}/lib
  	PATH_SUFFIXES build/qt5/src
)

set(POPPLER_QT5_LIBRARIES ${POPPLER_QT5_LIBRARY})

if (POPPLER_QT5_INCLUDE_DIR AND POPPLER_CPP_INCLUDE_DIR AND POPPLER_QT5_LIBRARIES)
  set(POPPLER_QT5_FOUND TRUE)
else ()
  set(POPPLER_QT5_FOUND FALSE)
endif ()

if (POPPLER_QT5_FOUND)
  if (NOT PopplerQt4_FIND_QUIETLY)
    message(STATUS "Found poppler-qt5: library: ${POPPLER_QT5_LIBRARIES}, include path: ${POPPLER_QT5_INCLUDE_DIR}; cpp include path: ${POPPLER_CPP_INCLUDE_DIR}")
  endif ()
else ()
  if (PopplerQt4_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find poppler-qt5, make sure both poppler-qt5 and poppler-cpp are installed")
  endif ()
endif ()

mark_as_advanced(POPPLER_QT5_INCLUDE_DIR POPPLER_QT5_LIBRARIES)
