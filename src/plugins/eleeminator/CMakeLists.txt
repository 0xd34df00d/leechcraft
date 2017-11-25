cmake_minimum_required (VERSION 3.3)
project (leechcraft_eleeminator)
include (InitLCPlugin)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (QTermWidget REQUIRED)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${QTERMWIDGET_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	eleeminator.cpp
	termtab.cpp
	processgraphbuilder.cpp
	processinfo.cpp
	xmlsettingsmanager.cpp
	closedialog.cpp
	colorschemesmanager.cpp
	)
set (FORMS
	closedialog.ui
	)
set (RESOURCES eleeminatorresources.qrc)
QtAddResources (RCCS ${RESOURCES})
QtWrapUi (UIS_H ${FORMS})
CreateTrs("eleeminator" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("eleeminator" "en;ru_RU" "${SRCS}" "${FORMS}" "eleeminatorsettings.xml")

add_library (leechcraft_eleeminator SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_eleeminator
	${LEECHCRAFT_LIBRARIES}
	${QTERMWIDGET_LIBRARIES}
	)
install (TARGETS leechcraft_eleeminator DESTINATION ${LC_PLUGINS_DEST})
install (FILES eleeminatorsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_eleeminator Widgets)
