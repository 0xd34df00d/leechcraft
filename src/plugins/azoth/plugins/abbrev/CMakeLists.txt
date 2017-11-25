cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_abbrev)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (ABBREV_SRCS
	abbrev.cpp
	abbrevsmanager.cpp
	abbreviation.cpp
	shortcutsmanager.cpp
	)

CreateTrs ("azoth_abbrev" "en;ru_RU" ABBREV_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_abbrev" "en;ru_RU" "${ABBREV_SRCS}" "" "")

add_library (leechcraft_azoth_abbrev SHARED
	${ABBREV_COMPILED_TRANSLATIONS}
	${ABBREV_SRCS}
	)
target_link_libraries (leechcraft_azoth_abbrev
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_abbrev DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_azoth_abbrev Core Widgets)
