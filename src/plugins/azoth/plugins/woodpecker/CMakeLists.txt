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

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (QtKOAuth REQUIRED)
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
set (SRCS
	core.cpp
	tweet.cpp
	twitdelegate.cpp
	twitterinterface.cpp
	twitterpage.cpp
	twitteruser.cpp
	woodpecker.cpp
	xmlsettingsmanager.cpp
	)
set (RESOURCES
	woodpeckerresources.qrc
	)
set (FORMS
	twitterpage.ui
	)
# CreateTrs ("woodpecker" "en;ru_RU" COMPILED_TRANSLATIONS)
# CreateTrsUpTarget("woodpecker" "en;ru_RU" "${SRCS}" "${FORMS}" "woodpeckersettings.xml")
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_azoth_woodpecker SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_azoth_woodpecker
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QJSON_LIBRARIES}
	${QTKOAUTH_LIBRARIES}
	)

install (TARGETS leechcraft_azoth_woodpecker DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES woodpeckersettings.xml DESTINATION ${LC_SETTINGS_DEST})
