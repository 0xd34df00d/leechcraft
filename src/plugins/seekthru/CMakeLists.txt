cmake_minimum_required (VERSION 3.3)
project (leechcraft_seekthru)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
)
set (SRCS
	seekthru.cpp
	core.cpp
	description.cpp
	xmlsettingsmanager.cpp
	searcherslist.cpp
	findproxy.cpp
	searchhandler.cpp
	wizardgenerator.cpp
	startupfirstpage.cpp
	tagsasker.cpp
	)
set (FORMS
	searcherslist.ui
	startupfirstpage.ui
	tagsasker.ui
	)
set (RESOURCES
	seekthruresources.qrc
	)
QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

CreateTrs("seekthru" "ar;be;de;en;eo;es;fr;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("seekthru" "en;ru_RU" "${SRCS}" "${FORMS}" "seekthrusettings.xml")

add_library (leechcraft_seekthru SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_seekthru
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_seekthru DESTINATION ${LC_PLUGINS_DEST})
install (FILES seekthrusettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_seekthru Widgets Xml)
