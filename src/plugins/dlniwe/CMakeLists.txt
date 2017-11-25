cmake_minimum_required (VERSION 3.3)
project (leechcraft_dlniwe)
include (InitLCPlugin)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (HUpnp REQUIRED)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${HUpnp_INCLUDE_DIR}
	)
set (SRCS
	dlniwe.cpp
	fspathsmanager.cpp
	xmlsettingsmanager.cpp
	)
set (RESOURCES dlniweresources.qrc)
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

#CreateTrs("dlniwe" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("dlniwe" "en;ru_RU" "${SRCS}" "${FORMS}" "dlniwesettings.xml")

add_library (leechcraft_dlniwe SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_dlniwe
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${HUpnp_LIBRARIES}
	)
install (TARGETS leechcraft_dlniwe DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/dlniwe DESTINATION ${LC_SHARE_DEST})
install (FILES dlniwesettings.xml DESTINATION ${LC_SETTINGS_DEST})
