# kQOAuth library search

if (QTKOAUTH_INCLUDE_DIR AND QTKOAUTH_LIBRARIES)
	# check cache
	set(QTKOAUTH_FOUND TRUE)
else ()
	if (NOT WIN32)
		find_package(PkgConfig)
		PKG_CHECK_MODULES(PC_QTKOAUTH QUIET kqoauth)
		set(QTKOAUTH_DEFINITIONS ${PC_QTKOAUTH_CFLAGS_OTHER})
	endif ()

	include(FindLibraryWithDebug)

	find_library_with_debug(QTKOAUTH_LIBRARIES
		WIN32_DEBUG_POSTFIX d
		NAMES kqoauth-qt5 kqoauth kqoauth0
		HINTS
			${QT_LIBRARY_DIR}
			${QTKOAUTH_DIR}
			${PC_QTKOAUTH_LIBDIR}
			${PC_QTKOAUTH_LIBRARY_DIRS}
		)

	find_path(QTKOAUTH_INCLUDE_DIR
		NAMES
			QtKOAuth/kqoauthrequest.h
		PATH_SUFFIXES
			QtKOAuth
		HINTS
			${QTKOAUTH_DIR}
			${INCLUDE_INSTALL_DIR}
			${KDE4_INCLUDE_DIR}
			${QT_INCLUDE_DIR}
			${QTKOAUTH_INCLUDE_WIN32}
			${PC_QTKOAUTH_INCLUDEDIR}
			${PC_QTKOAUTH_INCLUDE_DIRS}
		)

	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(QTKOAUTH DEFAULT_MSG QTKOAUTH_LIBRARIES QTKOAUTH_INCLUDE_DIR)

	message(STATUS "Found the QTKOAUTH libraries at ${QTKOAUTH_LIBRARIES} Includes at ${QTKOAUTH_INCLUDE_DIR}")

	mark_as_advanced(QTKOAUTH_INCLUDE_DIR QTKOAUTH_LIBRARIES)

endif ()
