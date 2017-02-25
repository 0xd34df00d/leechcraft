# - Try to find the QScintilla2 includes and library
# which defines
#
# QSCINTILLA_FOUND - system has QScintilla2
# QSCINTILLA_INCLUDE_DIR - where to find qextscintilla.h
# QSCINTILLA_LIBRARIES - the libraries to link against to use QScintilla
# QSCINTILLA_LIBRARY - where to find the QScintilla library (not for general use)

# copyright (c) 2007 Thomas Moenicke thomas.moenicke@kdemail.net
#
# Redistribution and use is allowed according to the terms of the FreeBSD license.

set(QSCINTILLA_FOUND FALSE)

find_path(QSCINTILLA_INCLUDE_DIR qsciglobal.h
	/usr/include /usr/include/qt5/Qsci /usr/local/include /usr/local/include/qt5/Qsci
	)

set(QSCINTILLA_NAMES ${QSCINTILLA_NAMES} qscintilla2 libqscintilla2)
find_library(QSCINTILLA_LIBRARY
	NAMES ${QSCINTILLA_NAMES}
	PATHS ${QT_LIBRARY_DIR}
	)

if (QSCINTILLA_LIBRARY AND QSCINTILLA_INCLUDE_DIR)

	set(QSCINTILLA_LIBRARIES ${QSCINTILLA_LIBRARY})
	set(QSCINTILLA_FOUND TRUE)

	if (CYGWIN)
		if(BUILD_SHARED_LIBS)
		# No need to define QSCINTILLA_USE_DLL here, because it's default for Cygwin.
		else()
		set (QSCINTILLA_DEFINITIONS -DQSCINTILLA_STATIC)
		endif()
	endif ()

endif ()

if (QSCINTILLA_FOUND)
  if (NOT QScintilla_FIND_QUIETLY)
	message(STATUS "Found QScintilla2: ${QSCINTILLA_LIBRARY}")
  endif ()
else ()
  if (QScintilla_FIND_REQUIRED)
	message(FATAL_ERROR "Could not find QScintilla library")
  endif ()
endif ()

mark_as_advanced(QSCINTILLA_INCLUDE_DIR QSCINTILLA_LIBRARY)

