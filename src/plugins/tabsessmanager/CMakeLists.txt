cmake_minimum_required (VERSION 3.3)
project (leechcraft_tabsessmanager)
include (InitLCPlugin)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	tabsessmanager.cpp
	restoresessiondialog.cpp
	recinfo.cpp
	sessionmenumanager.cpp
	sessionsmanager.cpp
	tabspropsmanager.cpp
	util.cpp
	unclosemanager.cpp
	)
set (FORMS
	restoresessiondialog.ui
	)
CreateTrs ("tabsessmanager" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("tabsessmanager" "en;ru_RU" "${SRCS}" "${FORMS}" "")

QtWrapUi (UIS_H ${FORMS})

add_library (leechcraft_tabsessmanager SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_tabsessmanager
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_tabsessmanager DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_tabsessmanager Widgets)
