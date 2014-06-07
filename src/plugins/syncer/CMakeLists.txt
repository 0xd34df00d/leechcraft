cmake_minimum_required (VERSION 2.8)
project (leechcraft_syncer)
include (InitLCPlugin OPTIONAL)

find_package (LibLaretz REQUIRED)

set (QT_USE_QTNETWORK TRUE)
include (${QT_USE_FILE})
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
set (TRANSLATIONS
	leechcraft_syncer_ru_RU.ts
	leechcraft_syncer_en.ts
	)
set (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_syncer_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_syncer_en.qm
	)

add_custom_command (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
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
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES syncersettings.xml DESTINATION ${LC_SETTINGS_DEST})
