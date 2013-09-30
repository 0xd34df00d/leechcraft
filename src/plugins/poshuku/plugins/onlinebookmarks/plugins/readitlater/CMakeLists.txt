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

set (READITLATER_HEADERS
	readitlater.h
	readitlaterservice.h
	readitlaterauthwidget.h
	readitlateraccount.h
	readitlaterapi.h
	)

set (READITLATER_FORMS readitlaterauthwidget.ui)

CreateTrs ("poshuku_onlinebookmarks_readitlater" "en;ru_RU;uk_UA" ONLINEBOOKMARKS_READITLATER_COMPILED_TRANSLATIONS)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (READITLATER_MOC_SRCS ${READITLATER_HEADERS})
endif ()
QT4_WRAP_UI (READITLATER_UIS_H ${READITLATER_FORMS})

set (READITLATER_RESOURCES readitlaterresources.qrc)
QT4_ADD_RESOURCES (READITLATER_RCCS ${READITLATER_RESOURCES})

add_library (leechcraft_poshuku_onlinebookmarks_readitlater SHARED
 	${ONLINEBOOKMARKS_READITLATER_COMPILED_TRANSLATIONS}
	${READITLATER_SRCS}
	${READITLATER_MOC_SRCS}
	${READITLATER_UIS_H}
	${READITLATER_RCCS}
	)

target_link_libraries (leechcraft_poshuku_onlinebookmarks_readitlater
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QJSON_LIBRARIES}
	)

install (TARGETS leechcraft_poshuku_onlinebookmarks_readitlater DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${ONLINEBOOKMARKS_READITLATER_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
