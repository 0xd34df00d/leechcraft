cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_autoidler)
include (InitLCPlugin)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (AUTOIDLER_SRCS
	autoidler.cpp
	xmlsettingsmanager.cpp
	3dparty/idle.cpp
	)
set (AUTOIDLER_RESOURCES azothautoidlerresources.qrc)
QtAddResources (AUTOIDLER_RCCS ${AUTOIDLER_RESOURCES})

if (APPLE)
	set (AUTOIDLER_SRCS ${AUTOIDLER_SRCS} 3dparty/idle_mac.cpp)
	set (ADDITIONAL_AUTOIDLER_LIBS "-framework ApplicationServices")
elseif (UNIX)
	set (AUTOIDLER_SRCS ${AUTOIDLER_SRCS} 3dparty/idle_x11.cpp)

	find_package (X11)

	if (X11_Xscreensaver_FOUND)
		add_definitions (-DHAVE_XSS)
		include_directories ("${X11_Xscreensaver_INCLUDE_PATH}")
		set (ADDITIONAL_AUTOIDLER_LIBS ${X11_Xscreensaver_LIB})
	endif ()
elseif (WIN32)
	set (AUTOIDLER_SRCS ${AUTOIDLER_SRCS} 3dparty/idle_win.cpp)
endif ()

add_definitions (-DUNICODE)

CreateTrs ("azoth_autoidler" "en;es;ro;ru_RU;uk_UA" AUTOIDLER_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_autoidler" "en;ru_RU" "${AUTOIDLER_SRCS}" "" "azothautoidlersettings.xml")

add_library (leechcraft_azoth_autoidler SHARED
	${AUTOIDLER_COMPILED_TRANSLATIONS}
	${AUTOIDLER_SRCS}
	${AUTOIDLER_MOC_SRCS}
	${AUTOIDLER_RCCS}
	)
target_link_libraries (leechcraft_azoth_autoidler
	${LEECHCRAFT_LIBRARIES}
	${ADDITIONAL_AUTOIDLER_LIBS}
	)
install (TARGETS leechcraft_azoth_autoidler DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothautoidlersettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (FILES ${AUTOIDLER_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})

FindQtLibs (leechcraft_azoth_autoidler Widgets)

if (UNIX AND NOT APPLE)
	FindQtLibs (leechcraft_azoth_autoidler X11Extras)
endif ()
