cmake_minimum_required (VERSION 2.8)
project (leechcraft_krigstask)
include (InitLCPlugin OPTIONAL)

find_package (X11 REQUIRED)

set (QT_USE_QTDECLARATIVE TRUE)
include (${QT_USE_FILE})
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
	)
#CreateTrs("krigstask" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("krigstask" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_krigstask SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_krigstask
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${X11_X11_LIB}
	${X11_Xcomposite_LIB}
	)
install (TARGETS leechcraft_krigstask DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/qml/krigstask DESTINATION ${LC_QML_DEST})
