cmake_minimum_required (VERSION 3.3)
project (leechcraft_intermutko)
include (InitLCPlugin)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	acceptlangwidget.cpp
	entriesdelegate.cpp
	intermutko.cpp
	localeentry.cpp
	localesmodel.cpp
	util.cpp
	xmlsettingsmanager.cpp
	)
set (FORMS
	acceptlangwidget.ui
	)
CreateTrs("intermutko" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("intermutko" "en;ru_RU" "${SRCS}" "${FORMS}" "intermutkosettings.xml")
QtWrapUi (UIS_H ${FORMS})

add_library (leechcraft_intermutko SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_intermutko
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_intermutko DESTINATION ${LC_PLUGINS_DEST})
install (FILES intermutkosettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_intermutko Network Widgets)
