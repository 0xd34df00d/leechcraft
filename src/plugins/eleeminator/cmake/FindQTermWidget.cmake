find_path(QTERMWIDGET_INCLUDE_DIR qtermwidget.h
	PATHS
		${QTERMWIDGET_PATH_INCLUDES}/
		/usr/include/
		/usr/local/include/
		/usr/local/include/
		/opt/local/include/
	PATH_SUFFIXES
		qtermwidget4
		qtermwidget5
	)

if (NOT USE_QT5)
	find_library(QTERMWIDGET_LIBRARIES
		NAMES
			qtermwidget4
			qtermwidget
		PATHS
			${QTERMWIDGET_PATH_LIB}
			/usr/lib/
			/usr/lib${LIB_SUFFIX}/
			/usr/local/lib/
			/usr/local/lib${LIB_SUFFIX}/
			/opt/local/lib/
		)
else ()
	find_library(QTERMWIDGET_LIBRARIES
		NAMES
			qtermwidget5
			qtermwidget
		PATHS
			${QTERMWIDGET_PATH_LIB}
			/usr/lib/
			/usr/lib${LIB_SUFFIX}/
			/usr/local/lib/
			/usr/local/lib${LIB_SUFFIX}/
			/opt/local/lib/
		)
endif ()

if (QTERMWIDGET_LIBRARIES AND QTERMWIDGET_INCLUDE_DIR)
	set (QTERMWIDGET_FOUND TRUE)
endif ()

message (STATUS "Found QTermWidget includes at ${QTERMWIDGET_INCLUDE_DIR}")
message (STATUS "Found QTermWidget libraries at ${QTERMWIDGET_LIBRARIES}")

if (NOT QTERMWIDGET_FOUND)
	if (QTermWidget_FIND_REQUIRED)
		message (FATAL_ERROR "Could not find QTermWidget library")
	endif ()
endif ()
