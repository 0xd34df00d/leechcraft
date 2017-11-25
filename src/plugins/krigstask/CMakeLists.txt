cmake_minimum_required (VERSION 3.3)
project (leechcraft_krigstask)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (X11 REQUIRED)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	krigstask.cpp
	windowsmodel.cpp
	taskbarproxy.cpp
	pagerwindow.cpp
	pagerwindowproxy.cpp
	)
CreateTrs("krigstask" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("krigstask" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_krigstask SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)

find_package (XCB COMPONENTS COMPOSITE XFIXES RENDER RENDERUTIL REQUIRED)

set (ADDITIONAL_LIBS
	${X11_X11_LIB}
	${X11_Xcomposite_LIB}
	${X11_Xfixes_LIB}
	XCB::COMPOSITE
	XCB::XFIXES
	XCB::RENDER
	XCB::RENDERUTIL
	)

target_link_libraries (leechcraft_krigstask
	${LEECHCRAFT_LIBRARIES}
	${ADDITIONAL_LIBS}
	)
install (TARGETS leechcraft_krigstask DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/qml5/krigstask DESTINATION ${LC_QML5_DEST})

FindQtLibs (leechcraft_krigstask QuickWidgets X11Extras)
