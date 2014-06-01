cmake_minimum_required (VERSION 2.8)
project (leechcraft_kbswitch)
include (InitLCPlugin OPTIONAL)

find_package (X11 REQUIRED)

if (NOT X11_Xlib_INCLUDE_PATH OR
	NOT X11_Xutil_INCLUDE_PATH OR
	NOT X11_Xkblib_INCLUDE_PATH OR
	NOT X11_Xaccessrules_INCLUDE_PATH)
	message (STATUS "Xlib: ${X11_Xlib_INCLUDE_PATH}")
	message (STATUS "Xutil: ${X11_Xutil_INCLUDE_PATH}")
	message (STATUS "XKBlib.h: ${X11_Xkblib_INCLUDE_PATH}")
	message (STATUS "XKBrules.h: ${X11_Xaccessrules_INCLUDE_PATH}")
	message (FATAL_ERROR "Could NOT find all required X11 libraries")
endif ()

set (QT_USE_QTDECLARATIVE TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${X11_Xlib_INCLUDE_PATH}
	${X11_Xutil_INCLUDE_PATH}
	${X11_Xkblib_INCLUDE_PATH}
	${X11_Xaccessrules_INCLUDE_PATH}
	)

set (SRCS
	kbswitch.cpp
	keyboardlayoutswitcher.cpp
	xmlsettingsmanager.cpp
	kbctl.cpp
	quarkproxy.cpp
	flagiconprovider.cpp
	rulesstorage.cpp
	layoutsconfigwidget.cpp
	optionsconfigwidget.cpp
	)
set (FORMS
	layoutsconfigwidget.ui
	)
set (RESOURCES kbswitchresources.qrc)
QT4_ADD_RESOURCES (RCCS ${RESOURCES})
QT4_WRAP_UI (UIS_H ${FORMS})

CreateTrs ("kbswitch" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("kbswitch" "en;ru_RU" "${SRCS}" "${FORMS}" "kbswitchsettings.xml")

add_library (leechcraft_kbswitch SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_kbswitch
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${X11_Xkbfile_LIB}
	)
install (TARGETS leechcraft_kbswitch DESTINATION ${LC_PLUGINS_DEST})
install (FILES kbswitchsettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/qml/kbswitch DESTINATION ${LC_QML_DEST})
