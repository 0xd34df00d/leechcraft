find_package (PkgConfig)
pkg_check_modules (UtilXParts REQUIRED IMPORTED_TARGET x11 xcb)

add_util_library (util-x11
	SRCS xwrapper.cpp
	DEPENDS PkgConfig::UtilXParts
	USES Widgets
	)
