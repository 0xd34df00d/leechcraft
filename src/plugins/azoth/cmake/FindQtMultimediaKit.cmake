# Try to find QtMultimediaKit.
#
# Defines:
# - QTMULTIMEDIAKIT_INCLUDE_DIRS
# - QTMULTIMEDIAKIT_LIBRARIES
# - QTMULTIMEDIAKIT_FOUND

if (QTMULTIMEDIAKIT_INCLUDE_DIRS AND QTMULTIMEDIAKIT_LIBRARIES)
	set (QTMULTIMEDIAKIT_FOUND TRUE)
else ()
	find_path (QTMULTIMEDIAKIT_INCLUDE_DIRS
		NAMES
			qaudio.h
		PATHS
			/usr/include/QtMultimediaKit
			/usr/local/include/QtMultimediaKit
			/usr/include/qt4/QtMultimediaKit
			/usr/local/include/qt4/QtMultimediaKit
		)

	find_library (QTMULTIMEDIAKIT_LIBRARIES
		NAMES
			QtMultimediaKit
		PATHS
			/usr/lib
			/usr/local/lib
			/usr/lib/qt4
			/usr/local/lib/qt4
	)

	if (QTMULTIMEDIAKIT_INCLUDE_DIRS AND QTMULTIMEDIAKIT_LIBRARIES)
		set (QTMULTIMEDIAKIT_FOUND TRUE)

		message (STATUS "Found QtMultimediaKit: includes at ${QTMULTIMEDIAKIT_INCLUDE_DIRS} and libraries at ${QTMULTIMEDIAKIT_LIBRARIES}")
	else ()
		message (FATAL_ERROR "Could NOT find QtMultimediaKit")
	endif ()
endif ()
