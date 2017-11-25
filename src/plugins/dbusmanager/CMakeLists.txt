cmake_minimum_required (VERSION 3.3)
project (leechcraft_dbusmanager)
include (InitLCPlugin)

include_directories (${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
	dbusmanager.cpp
	core.cpp
	iinfoadaptor.cpp
	xmlsettingsmanager.cpp
	general.cpp
	generaladaptor.cpp
	tasks.cpp
	tasksadaptor.cpp
	webfilestorageadaptor.cpp
	)
set (RESOURCES
	dbusmanagerresources.qrc
	)
CreateTrs ("dbusmanager" "ar;de;en;eo;es;fr;it;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("dbusmanager" "en;ru_RU" "${SRCS}" "" "dbusmanagersettings.xml")

QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_dbusmanager SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_dbusmanager
		${LEECHCRAFT_LIBRARIES}
		)
install (TARGETS leechcraft_dbusmanager DESTINATION ${LC_PLUGINS_DEST})
install (FILES dbusmanagersettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_dbusmanager DBus Gui)
