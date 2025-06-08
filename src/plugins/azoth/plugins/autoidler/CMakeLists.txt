cmake_minimum_required (VERSION 3.10)
project (azoth_autoidler)
include (InitLCPlugin NO_POLICY_SCOPE)

if (UNIX AND NOT APPLE)
	find_package (PkgConfig)
	pkg_check_modules (XSS REQUIRED IMPORTED_TARGET xscrnsaver)

	set (X11_IMPL 3dparty/idle_x11.cpp)
	set (X11_XSS_LIB PkgConfig::XSS)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		autoidler.cpp
		3dparty/idle.cpp
		${X11_IMPL}
		$<$<PLATFORM_ID:Win32>:3dparty/idle_win.cpp>
		$<$<PLATFORM_ID:Darwin>:3dparty/idle_mac.cpp>
	SETTINGS azothautoidlersettings.xml
	QT_COMPONENTS Widgets
	LINK_LIBRARIES
		$<$<PLATFORM_ID:Darwin>:"-framework ApplicationServices">
		${X11_XSS_LIB}
	INSTALL_SHARE
	)
