cmake_minimum_required (VERSION 2.8)
project (leechcraft_launchy)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTDECLARATIVE TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	launchy.cpp
	fsdisplayer.cpp
	itemssortfilterproxymodel.cpp
	favoritesmanager.cpp
	quarkmanager.cpp
	itemimageprovider.cpp
	syspathitemprovider.cpp
	recentmanager.cpp
	)
set (RESOURCES resources.qrc)
QT4_ADD_RESOURCES (RCCS ${RESOURCES})
CreateTrs ("launchy" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("launchy" "en;ru_RU" "${SRCS}" "" "")

add_library (leechcraft_launchy SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_launchy
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_launchy DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/qml/launchy DESTINATION ${LC_QML_DEST})
