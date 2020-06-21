cmake_minimum_required (VERSION 3.3)
project (leechcraft_rosenthal)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (Hunspell REQUIRED IMPORTED_TARGET hunspell>=1.5.1)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	rosenthal.cpp
	checker.cpp
	knowndictsmanager.cpp
	xmlsettingsmanager.cpp
	)
CreateTrs("rosenthal" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("rosenthal" "en;ru_RU" "${SRCS}" "${FORMS}" "rosenthalsettings.xml")

add_library (leechcraft_rosenthal SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_rosenthal
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	PkgConfig::Hunspell
	)
install (TARGETS leechcraft_rosenthal DESTINATION ${LC_PLUGINS_DEST})
install (FILES rosenthalsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_rosenthal Gui)
