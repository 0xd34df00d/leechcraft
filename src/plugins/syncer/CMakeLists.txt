cmake_minimum_required (VERSION 3.3)
project (leechcraft_syncer)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (LibLaretz REQUIRED)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LARETZ_INCLUDE_DIRS}
	)
set (SRCS
	syncer.cpp
	xmlsettingsmanager.cpp
	syncablemanager.cpp
	singlesyncable.cpp
	)

CreateTrs ("syncer" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("syncer" "en;ru_RU" "${SRCS}" "${FORMS}" "syncersettings.xml")

add_library (leechcraft_syncer SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
target_link_libraries (leechcraft_syncer
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${LARETZ_LIBRARIES}
	)
install (TARGETS leechcraft_syncer DESTINATION ${LC_PLUGINS_DEST})
install (FILES syncersettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_syncer Network Gui)
