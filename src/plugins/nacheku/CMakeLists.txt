cmake_minimum_required (VERSION 3.3)
project (leechcraft_nacheku)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	nacheku.cpp
	xmlsettingsmanager.cpp
	clipboardwatcher.cpp
	directorywatcher.cpp
	)

CreateTrs ("nacheku" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("nacheku" "en;ru_RU" "${SRCS}" "" "nachekusettings.xml")

add_library (leechcraft_nacheku SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_nacheku
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_nacheku DESTINATION ${LC_PLUGINS_DEST})
install (FILES nachekusettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_nacheku Core Widgets)
