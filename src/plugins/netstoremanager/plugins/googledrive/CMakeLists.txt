cmake_minimum_required (VERSION 2.8)
project (leechcraft_netstoremanager_googledrive)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (QJSON REQUIRED)
if (NOT WIN32)
	find_package (MAGIC REQUIRED)
	if (MAGIC_FOUND)
		add_definitions (-DHAVE_MAGIC)
	endif ()
endif ()

include_directories (${NETSTOREMANAGER_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QJSON_INCLUDE_DIR}
	${MAGIC_INCLUDE_DIR}
	)

set (GOOGLEDRIVE_SRCS
	account.cpp
	authmanager.cpp
	core.cpp
	drivemanager.cpp
	googledrive.cpp
	uploadmanager.cpp
	xmlsettingsmanager.cpp
	)
set (GOOGLEDRIVE_HEADERS
	account.h
	authmanager.h
	core.h
	drivemanager.h
	googledrive.h
	uploadmanager.h
	xmlsettingsmanager.h
	)

set (GOOGLEDRIVE_RESOURCES netstoremanagergoogledriveresources.qrc)

CreateTrs ("netstoremanager_googledrive" "en;ru_RU" GOOGLEDRIVE_COMPILED_TRANSLATIONS)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (GOOGLEDRIVE_MOC_SRCS ${GOOGLEDRIVE_HEADERS})
endif ()
QT4_ADD_RESOURCES (GOOGLEDRIVE_RCCS ${GOOGLEDRIVE_RESOURCES})

add_library (leechcraft_netstoremanager_googledrive SHARED
	${GOOGLEDRIVE_COMPILED_TRANSLATIONS}
	${GOOGLEDRIVE_SRCS}
	${GOOGLEDRIVE_MOC_SRCS}
	${GOOGLEDRIVE_UIS_H}
	${GOOGLEDRIVE_RCCS}
	)
target_link_libraries (leechcraft_netstoremanager_googledrive
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QJSON_LIBRARIES}
	${MAGIC_LIBRARIES}
	)

install (TARGETS leechcraft_netstoremanager_googledrive DESTINATION ${LC_PLUGINS_DEST})
install (FILES nsmgoogledrivesettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (FILES ${GOOGLEDRIVE_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
