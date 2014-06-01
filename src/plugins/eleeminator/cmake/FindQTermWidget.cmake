find_path(QTERMWIDGET_INCLUDE_DIR qtermwidget.h
	PATHS
		${QTERMWIDGET_PATH_INCLUDES}/
		/usr/include/
		/usr/local/include/
		/usr/local/include/
		/opt/local/include/
	PATH_SUFFIXES
		qtermwidget4
	)

find_library(QTERMWIDGET_LIBRARIES NAMES qtermwidget libqtermwidget qtermwidget4 libqtermwidget4
    PATHS
		${QTERMWIDGET_PATH_LIB}
		/usr/lib/
		/usr/lib${LIB_SUFFIX}/
		/usr/local/lib/
		/usr/local/lib${LIB_SUFFIX}/
		/opt/local/lib/
	)

if (QTERMWIDGET_LIBRARIES AND QTERMWIDGET_INCLUDE_DIR)
	set (QTERMWIDGET_FOUND TRUE)
endif ()

if (QTERMWIDGET_FOUND)
	message (STATUS "Found QTermWidget: includes at ${QTERMWIDGET_INCLUDE_DIR} and libs at ${QTERMWIDGET_LIBRARIES}")
else ()
	if (QTermWidget_FIND_REQUIRED)
		message (FATAL_ERROR "Could not find QTermWidget library")
	endif ()
endif ()
