cmake_minimum_required (VERSION 2.8)
project (leechcraft_xproxy)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTNETWORK TRUE)
include (${QT_USE_FILE})
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
	)
set (FORMS
	proxiesconfigwidget.ui
	)
QT4_WRAP_UI (UIS_H ${FORMS})
CreateTrs ("xproxy" "en;ru_RU" COMPILED_TRANSLATIONS)

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
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
