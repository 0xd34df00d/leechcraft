cmake_minimum_required (VERSION 3.3)
project (leechcraft_popishu)
include (InitLCPlugin NO_POLICY_SCOPE)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package (QScintilla)
if (NOT QSCINTILLA_FOUND)
	message (ERROR "QScintilla not found, unable to build Popishu :(")
endif ()

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QSCINTILLA_INCLUDE_DIR}
	)
set (SRCS
	popishu.cpp
	editorpage.cpp
	xmlsettingsmanager.cpp
	replacedialog.cpp
	)
set (RESOURCES
	popishuresources.qrc
	)
set (FORMS
	editorpage.ui
	replacedialog.ui
	)
CreateTrs ("popishu" "de;en;es;fr;it;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("popishu" "en;ru_RU" "${SRCS}" "${FORMS}" "popishusettings.xml")

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_popishu SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${HEADERS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_popishu
	${LEECHCRAFT_LIBRARIES}
	${QSCINTILLA_LIBRARIES}
	)
install (TARGETS leechcraft_popishu DESTINATION ${LC_PLUGINS_DEST})
install (FILES popishusettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_popishu Widgets)
