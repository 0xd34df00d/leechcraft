cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_autoidler)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${AZOTH_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

if (APPLE)
	set (ADDITIONAL_SRCS 3dparty/idle_mac.cpp)
	set (ADDITIONAL_AUTOIDLER_LIBS "-framework ApplicationServices")
elseif (UNIX)
	set (ADDITIONAL_SRCS 3dparty/idle_x11.cpp)
	set (ADDITIONAL_QT_COMPONENTS X11Extras)

	find_package (X11)

	if (X11_Xscreensaver_FOUND)
		add_definitions (-DHAVE_XSS)
		include_directories ("${X11_Xscreensaver_INCLUDE_PATH}")
		set (ADDITIONAL_AUTOIDLER_LIBS ${X11_Xscreensaver_LIB})
	endif ()
elseif (WIN32)
	set (ADDITIONAL_SRCS 3dparty/idle_win.cpp)
endif ()

LC_DEFINE_PLUGIN (azoth_autoidler
	SRCS
		autoidler.cpp
		xmlsettingsmanager.cpp
		3dparty/idle.cpp
		${ADDITIONAL_SRCS}
	SETTINGS azothautoidlersettings.xml
	QT_COMPONENTS Widgets ${ADDITIONAL_QT_COMPONENTS}
	LINK_LIBRARIES ${ADDITIONAL_AUTOIDLER_LIBS}
	TS_LANGUAGES en ro ru_RU
	INSTALL_SHARE
	)
