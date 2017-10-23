cmake_minimum_required (VERSION 3.3)
project (leechcraft_scroblibre)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	scroblibre.cpp
	xmlsettingsmanager.cpp
	accountsmanager.cpp
	authmanager.cpp
	singleaccauth.cpp
	util.cpp
	submitinfo.cpp
	)
CreateTrs("scroblibre" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("scroblibre" "en;ru_RU" "${SRCS}" "${FORMS}" "scroblibresettings.xml")

add_library (leechcraft_scroblibre SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_scroblibre
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_scroblibre DESTINATION ${LC_PLUGINS_DEST})
install (FILES scroblibresettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_scroblibre Gui Network)
