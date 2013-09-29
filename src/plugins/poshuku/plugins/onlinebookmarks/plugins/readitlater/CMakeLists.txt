set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (QJSON REQUIRED)

include_directories (${ONLINEBOOKMARKS_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${QJSON_INCLUDE_DIR}
	)

set (READITLATER_SRCS
	readitlater.cpp
	readitlaterservice.cpp
	readitlaterauthwidget.cpp
	readitlateraccount.cpp
	readitlaterapi.cpp
	)

set (READITLATER_FORMS readitlaterauthwidget.ui)

set (READITLATER_RESOURCES onlinebookmarksreaditlater.qrc)
CreateTrs ("poshuku_onlinebookmarks_readitlater" "en;ru_RU;uk_UA" ONLINEBOOKMARKS_READITLATER_COMPILED_TRANSLATIONS)

QtWrapUi (READITLATER_UIS_H ${READITLATER_FORMS})
QtAddResources (READITLATER_RCCS ${READITLATER_RESOURCES})

set (READITLATER_RESOURCES readitlaterresources.qrc)
QtAddResources (READITLATER_RCCS ${READITLATER_RESOURCES})

add_library (leechcraft_poshuku_onlinebookmarks_readitlater SHARED
 	${ONLINEBOOKMARKS_READITLATER_COMPILED_TRANSLATIONS}
	${READITLATER_SRCS}
	${READITLATER_UIS_H}
	${READITLATER_RCCS}
	)

FindQtLibs (leechcraft_poshuku_onlinebookmarks_readitlater Network)

target_link_libraries (leechcraft_poshuku_onlinebookmarks_readitlater
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QJSON_LIBRARIES}
	)

install (TARGETS leechcraft_poshuku_onlinebookmarks_readitlater DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${ONLINEBOOKMARKS_READITLATER_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
