cmake_minimum_required (VERSION 3.10)
project (azoth_autoidler)
include (InitLCPlugin NO_POLICY_SCOPE)

if (UNIX)
	find_package (PkgConfig)
	pkg_check_modules (XSS REQUIRED IMPORTED_TARGET xscrnsaver)

	set (X11_IMPL 3dparty/idle_x11.cpp)
	set (X11_XSS_LIB PkgConfig::XSS)
	set (QT_X11_EXTRA_LIBRARY X11Extras)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		autoidler.cpp
		xmlsettingsmanager.cpp
		3dparty/idle.cpp
		${X11_IMPL}
		$<$<PLATFORM_ID:Win32>:3dparty/idle_win.cpp>
		$<$<PLATFORM_ID:Darwin>:3dparty/idle_mac.cpp>
	SETTINGS azothautoidlersettings.xml
	QT_COMPONENTS Widgets ${QT_X11_EXTRA_LIBRARY}
	LINK_LIBRARIES
		$<$<PLATFORM_ID:Darwin>:"-framework ApplicationServices">
		${X11_XSS_LIB}
	INSTALL_SHARE
	)
