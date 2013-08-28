set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories (${ONLINEBOOKMARKS_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	)

set (DELICIOUS_SRCS
	delicious.cpp
	deliciousauthwidget.cpp
	deliciousservice.cpp
	deliciousaccount.cpp
	deliciousapi.cpp
	)

set (DELICIOUS_HEADERS
	delicious.h
	deliciousauthwidget.h
	deliciousservice.h
	deliciousaccount.h
	deliciousapi.h
	)

set (DELICIOUS_FORMS deliciousauthwidget.ui)

set (DELICIOUS_RESOURCES onlinebookmarksdelicious.qrc)
CreateTrs ("poshuku_onlinebookmarks_delicious" "en;ru_RU;uk_UA" ONLINEBOOKMARKS_DELICIOUS_COMPILED_TRANSLATIONS)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (DELICIOUS_MOC_SRCS ${DELICIOUS_HEADERS})
endif ()
QT4_WRAP_UI (DELICIOUS_UIS_H ${DELICIOUS_FORMS})
QT4_ADD_RESOURCES (DELICIOUS_RCCS ${DELICIOUS_RESOURCES})
set (DELICIOUS_RESOURCES deliciousresources.qrc)
QT4_ADD_RESOURCES (DELICIOUS_RCCS ${DELICIOUS_RESOURCES})

add_library (leechcraft_poshuku_onlinebookmarks_delicious SHARED
 	${ONLINEBOOKMARKS_DELICIOUS_COMPILED_TRANSLATIONS}
	${DELICIOUS_SRCS}
	${DELICIOUS_MOC_SRCS}
	${DELICIOUS_UIS_H}
	${DELICIOUS_RCCS}
	)

target_link_libraries (leechcraft_poshuku_onlinebookmarks_delicious
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_poshuku_onlinebookmarks_delicious RUNTIME DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${ONLINEBOOKMARKS_DELICIOUS_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
