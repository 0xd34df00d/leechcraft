cmake_minimum_required (VERSION 3.3)
project (leechcraft_rosenthal)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (Hunspell REQUIRED)
include_directories (${HUNSPELL_INCLUDE_DIR})

if (MSVC)
	add_definitions (-DWIN32 -DHUNSPELL_STATIC)
endif ()

include (cmake/CheckHunspellOverloads.cmake)
if (HUNSPELL_HAS_CXX_OVERLOADS)
	add_definitions (-DHUNSPELL_HAS_CXX_OVERLOADS)
endif ()

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
	${HUNSPELL_LIBRARIES}
	)
install (TARGETS leechcraft_rosenthal DESTINATION ${LC_PLUGINS_DEST})
install (FILES rosenthalsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_rosenthal Gui)
