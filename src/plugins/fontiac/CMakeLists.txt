cmake_minimum_required (VERSION 3.3)
project (leechcraft_fontiac)
include (InitLCPlugin)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	fontiac.cpp
	substsmanager.cpp
	xmlsettingsmanager.cpp
	)
CreateTrs("fontiac" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("fontiac" "en;ru_RU" "${SRCS}" "${FORMS}" "fontiacsettings.xml")

add_library (leechcraft_fontiac SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_fontiac
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_fontiac DESTINATION ${LC_PLUGINS_DEST})
install (FILES fontiacsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_fontiac Core)
