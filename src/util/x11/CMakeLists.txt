set (X11_SRCS
	xwrapper.cpp
	)

find_package (PkgConfig)
pkg_check_modules (UtilXParts REQUIRED IMPORTED_TARGET x11 xcb)

add_util_library (util-x11
	SOURCES ${X11_SRCS}
	DEPENDS PkgConfig::UtilXParts
	USES Widgets
	)
