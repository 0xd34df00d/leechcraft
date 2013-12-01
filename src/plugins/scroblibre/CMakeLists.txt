cmake_minimum_required (VERSION 2.8)
project (leechcraft_scroblibre)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	scroblibre.cpp
	accountsmanager.cpp
	xmlsettingsmanager.cpp
	singleaccauth.cpp
	util.cpp
	)
#CreateTrs("scroblibre" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("scroblibre" "en;ru_RU" "${SRCS}" "${FORMS}" "scroblibresettings.xml")

add_library (leechcraft_scroblibre SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_scroblibre
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_scroblibre DESTINATION ${LC_PLUGINS_DEST})
install (FILES scroblibresettings.xml DESTINATION ${LC_SETTINGS_DEST})
