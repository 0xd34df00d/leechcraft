cmake_minimum_required (VERSION 3.3)
project (leechcraft_lmp_dumbsync)
include (InitLCPlugin)

include_directories (${LMP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (DUMBSYNC_SRCS
	dumbsync.cpp
	xmlsettingsmanager.cpp
	)

CreateTrs ("lmp_dumbsync" "en;ru_RU" DUMBSYNC_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("lmp_dumbsync" "en;ru_RU" "${DUMBSYNC_SRCS}" "${DUMBSYNC_FORMS}" "lmpdumbsyncsettings.xml")

add_library (leechcraft_lmp_dumbsync SHARED
	${DUMBSYNC_COMPILED_TRANSLATIONS}
	${DUMBSYNC_SRCS}
	)
target_link_libraries (leechcraft_lmp_dumbsync
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_lmp_dumbsync DESTINATION ${LC_PLUGINS_DEST})
install (FILES lmpdumbsyncsettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (FILES ${DUMBSYNC_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})

FindQtLibs (leechcraft_lmp_dumbsync Concurrent Gui)
