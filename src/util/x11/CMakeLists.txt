set (X11_SRCS
	xwrapper.cpp
	)

find_package (PkgConfig)
pkg_check_modules (UtilXParts REQUIRED IMPORTED_TARGET x11 xcb)

foreach (SRC ${X11_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/x11/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_util_library (util-x11
	SOURCES ${X11_SRCS}
	DEPENDS PkgConfig::UtilXParts
	USES Widgets
	)
