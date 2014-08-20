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
set (DELICIOUS_FORMS deliciousauthwidget.ui)
set (DELICIOUS_RESOURCES deliciousresources.qrc)

CreateTrs ("poshuku_onlinebookmarks_delicious" "en;ru_RU;uk_UA" ONLINEBOOKMARKS_DELICIOUS_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku_onlinebookmarks_delicious" "en;ru_RU" "${DELICIOUS_SRCS}" "${DELICIOUS_FORMS}" "")

QtWrapUi (DELICIOUS_UIS_H ${DELICIOUS_FORMS})
QtAddResources (DELICIOUS_RCCS ${DELICIOUS_RESOURCES})

add_library (leechcraft_poshuku_onlinebookmarks_delicious SHARED
 	${ONLINEBOOKMARKS_DELICIOUS_COMPILED_TRANSLATIONS}
	${DELICIOUS_SRCS}
	${DELICIOUS_UIS_H}
	${DELICIOUS_RCCS}
	)

target_link_libraries (leechcraft_poshuku_onlinebookmarks_delicious
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_poshuku_onlinebookmarks_delicious DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_poshuku_onlinebookmarks_delicious Widgets)
