cmake_minimum_required (VERSION 3.3)
project (leechcraft_mellonetray)
include (InitLCPlugin)

find_package (X11 REQUIRED)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	mellonetray.cpp
	traymodel.cpp
	iconhandler.cpp
	)
CreateTrs("mellonetray" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("mellonetray" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_mellonetray SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_mellonetray
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${X11_X11_LIB}
	${X11_Xdamage_LIB}
	)
install (TARGETS leechcraft_mellonetray DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/qml5/mellonetray DESTINATION ${LC_QML5_DEST})

FindQtLibs (leechcraft_mellonetray Quick Widgets X11Extras)
