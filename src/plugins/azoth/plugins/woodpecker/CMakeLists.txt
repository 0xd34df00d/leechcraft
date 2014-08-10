cmake_minimum_required (VERSION 2.8)
if (COMMAND cmake_policy)
	cmake_policy (SET CMP0003 NEW)
endif ()

option (ENABLE_AZOTH_WOODPECKER_VERBOSE_LOG "Debug protocol messages & similar stuff with increased verbosity" OFF)

if (ENABLE_AZOTH_WOODPECKER_VERBOSE_LOG)
	add_definitions (-DWP_DEBUG)
endif ()

project (leechcraft_azoth_woodpecker)
include (InitLCPlugin OPTIONAL)

find_package (QtKOAuth REQUIRED)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (QJSON REQUIRED)

set (QT_USE_QTNETWORK TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QT_INCLUDE_DIR}
	${QJSON_INCLUDE_DIR}
	${QTKOAUTH_INCLUDE_DIR}
	)
set (WOODPECKER_SRCS
	core.cpp
	tweet.cpp
	twitdelegate.cpp
	twitterinterface.cpp
	twitterpage.cpp
	twitteruser.cpp
	woodpecker.cpp
	xmlsettingsmanager.cpp
	)
set (WOODPECKER_RESOURCES
	woodpeckerresources.qrc
	)
set (WOODPECKER_FORMS
	twitterpage.ui
	)
CreateTrs ("azoth_woodpecker" "en;ru_RU" WOODPECKER_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_woodpecker" "en;ru_RU" "${WOODPECKER_SRCS}" "${WOODPECKER_FORMS}" "azothwoodpeckersettings.xml")
QT4_WRAP_UI (WOODPECKER_UIS_H ${WOODPECKER_FORMS})
QT4_ADD_RESOURCES (WOODPECKER_RCCS ${WOODPECKER_RESOURCES})

add_library (leechcraft_azoth_woodpecker SHARED
	${WOODPECKER_COMPILED_TRANSLATIONS}
	${WOODPECKER_SRCS}
	${WOODPECKER_UIS_H}
	${WOODPECKER_RCCS}
	)
target_link_libraries (leechcraft_azoth_woodpecker
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QJSON_LIBRARIES}
	${QTKOAUTH_LIBRARIES}
	)

install (TARGETS leechcraft_azoth_woodpecker DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothwoodpeckersettings.xml DESTINATION ${LC_SETTINGS_DEST})
