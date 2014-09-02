cmake_minimum_required (VERSION 2.8)
project (leechcraft_netstoremanager_dbox)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (NOT USE_QT5)
	find_package (QJSON REQUIRED)
else ()
	unset (QJSON_INCLUDE_DIR)
	unset (QJSON_LIBRARIES)
endif ()

include_directories (${NETSTOREMANAGER_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QJSON_INCLUDE_DIR}
	)

set (DBOX_SRCS
	account.cpp
	authmanager.cpp
	chunkiodevice.cpp
	core.cpp
	drivemanager.cpp
	dropbox.cpp
	uploadmanager.cpp
	xmlsettingsmanager.cpp
	)

set (DBOX_RESOURCES netstoremanagerdropboxresources.qrc)

QtAddResources (DBOX_RCCS ${DBOX_RESOURCES})

add_library (leechcraft_netstoremanager_dbox SHARED
	${DBOX_SRCS}
	${DBOX_RCCS}
	)
target_link_libraries (leechcraft_netstoremanager_dbox
	${LEECHCRAFT_LIBRARIES}
	${QJSON_LIBRARIES}
	)

install (TARGETS leechcraft_netstoremanager_dbox DESTINATION ${LC_PLUGINS_DEST})
install (FILES nsmdropboxsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_netstoremanager_dbox Core)
