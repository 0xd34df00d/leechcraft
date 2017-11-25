cmake_minimum_required (VERSION 3.3)
project (leechcraft_launchy)
include (InitLCPlugin)

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
QtAddResources (RCCS ${RESOURCES})
CreateTrs ("launchy" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("launchy" "en;ru_RU" "${SRCS}" "" "")

add_library (leechcraft_launchy SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_launchy
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_launchy DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/qml5/launchy DESTINATION ${LC_QML5_DEST})

FindQtLibs (leechcraft_launchy QuickWidgets)
