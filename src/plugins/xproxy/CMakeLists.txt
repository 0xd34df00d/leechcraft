cmake_minimum_required (VERSION 3.3)
project (leechcraft_xproxy)
include (InitLCPlugin)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	xproxy.cpp
	proxyfactory.cpp
	xmlsettingsmanager.cpp
	proxiesconfigwidget.cpp
	proxyconfigdialog.cpp
	proxiesstorage.cpp
	structures.cpp
	editurlsdialog.cpp
	editurldialog.cpp
	editlistsdialog.cpp
	scriptsmanager.cpp
	urllistscript.cpp
	)
set (FORMS
	proxiesconfigwidget.ui
	proxyconfigdialog.ui
	editurlsdialog.ui
	editurldialog.ui
	editlistsdialog.ui
	)
QtWrapUi (UIS_H ${FORMS})

CreateTrs ("xproxy" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("xproxy" "en;ru_RU" "${SRCS}" "${FORMS}" "xproxysettings.xml")

add_library (leechcraft_xproxy SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_xproxy
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_xproxy DESTINATION ${LC_PLUGINS_DEST})
install (FILES xproxysettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/scripts/xproxy DESTINATION ${LC_SCRIPTS_DEST})

FindQtLibs (leechcraft_xproxy Network Widgets)
