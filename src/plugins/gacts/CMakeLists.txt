cmake_minimum_required (VERSION 3.10)
project (gacts)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qxt)

if (UNIX AND NOT APPLE)
	find_package (PkgConfig)
	pkg_check_modules (X11 REQUIRED IMPORTED_TARGET x11)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		gacts.cpp
		3rdparty/qxt/qxtglobalshortcut.cpp
		$<$<PLATFORM_ID:Linux,FreeBSD>:3rdparty/qxt/qxtglobalshortcut_x11.cpp>
		$<$<PLATFORM_ID:Darwin>:3rdparty/qxt/qxtglobalshortcut_mac.cpp>
		$<$<PLATFORM_ID:Windows>:3rdparty/qxt/qxtglobalshortcut_win.cpp>
	QT_COMPONENTS Core
	LINK_LIBRARIES
		$<$<PLATFORM_ID:Linux,FreeBSD>:PkgConfig::X11>
		$<$<PLATFORM_ID:Darwin>:"-framework CoreFoundation -framework Carbon">
	)
