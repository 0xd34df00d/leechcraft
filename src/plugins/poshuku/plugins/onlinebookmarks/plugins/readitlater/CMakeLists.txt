include_directories (${ONLINEBOOKMARKS_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	)

set (READITLATER_SRCS
	readitlater.cpp
	readitlaterservice.cpp
	readitlaterauthwidget.cpp
	readitlateraccount.cpp
	readitlaterapi.cpp
	)
set (READITLATER_FORMS readitlaterauthwidget.ui)
set (READITLATER_RESOURCES readitlaterresources.qrc)

CreateTrs ("poshuku_onlinebookmarks_readitlater" "en;ru_RU;uk_UA" ONLINEBOOKMARKS_READITLATER_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku_onlinebookmarks_readitlater" "en;ru_RU" "${READITLATER_SRCS}" "${READITLATER_FORMS}" "")

QtWrapUi (READITLATER_UIS_H ${READITLATER_FORMS})
QtAddResources (READITLATER_RCCS ${READITLATER_RESOURCES})

add_library (leechcraft_poshuku_onlinebookmarks_readitlater SHARED
 	${ONLINEBOOKMARKS_READITLATER_COMPILED_TRANSLATIONS}
	${READITLATER_SRCS}
	${READITLATER_UIS_H}
	${READITLATER_RCCS}
	)

target_link_libraries (leechcraft_poshuku_onlinebookmarks_readitlater
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_poshuku_onlinebookmarks_readitlater DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_poshuku_onlinebookmarks_readitlater Widgets Network)
