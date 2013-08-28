cmake_minimum_required (VERSION 2.8)
project (leechcraft_knowhow)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	knowhow.cpp
	xmlsettingsmanager.cpp
	tipdialog.cpp
	)
set (HEADERS
	knowhow.h
	xmlsettingsmanager.h
	tipdialog.h
	)
set (FORMS
	tipdialog.ui
	)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif ()
QT4_WRAP_UI (UIS_H ${FORMS})

add_library (leechcraft_knowhow SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_knowhow
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_knowhow RUNTIME DESTINATION ${LC_PLUGINS_DEST})
install (FILES knowhowsettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/knowhow DESTINATION ${LC_SHARE_DEST})