set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	)

set (ONLINEBOOKMARKS_SRCS
	onlinebookmarks.cpp
	xmlsettingsmanager.cpp
	core.cpp
	accountssettings.cpp
	pluginmanager.cpp
	)
set (ONLINEBOOKMARKS_FORMS
	accountssettings.ui
	)

set (ONLINEBOOKMARKS_RESOURCES poshukuonlinebookmarksresources.qrc
	)

CreateTrs ("poshuku_onlinebookmarks" "de;en;fr;it;ru_RU;uk_UA" ONLINEBOOKMARKS_COMPILED_TRANSLATIONS)

QtWrapUi (ONLINEBOOKMARKS_UIS_H ${ONLINEBOOKMARKS_FORMS})
QtAddResources (ONLINEBOOKMARKS_RCCS ${ONLINEBOOKMARKS_RESOURCES})

add_library (leechcraft_poshuku_onlinebookmarks SHARED
 	${ONLINEBOOKMARKS_COMPILED_TRANSLATIONS}
	${ONLINEBOOKMARKS_SRCS}
	${ONLINEBOOKMARKS_UIS_H}
	${ONLINEBOOKMARKS_RCCS}
	)

FindQtLibs (leechcraft_poshuku_onlinebookmarks Core Gui)

target_link_libraries (leechcraft_poshuku_onlinebookmarks
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_poshuku_onlinebookmarks DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${ONLINEBOOKMARKS_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES poshukuonlinebookmarkssettings.xml DESTINATION ${LC_SETTINGS_DEST})

set (ONLINEBOOKMARKS_INCLUDE_DIR ${CURRENT_SOURCE_DIR})

option (ENABLE_ONLINEBOOKMARKS_READITLATER "Build \"Read It Later\" support for OnlineBookmarks" ON)
option (ENABLE_ONLINEBOOKMARKS_DELICIOUS "Build \"Delicious\" support for OnlineBookmarks" ON)

if (ENABLE_ONLINEBOOKMARKS_READITLATER)
	add_subdirectory (plugins/readitlater)
endif ()

if (ENABLE_ONLINEBOOKMARKS_DELICIOUS)
	add_subdirectory (plugins/delicious)
endif ()
