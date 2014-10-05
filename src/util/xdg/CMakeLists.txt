set (XDG_SRCS
	desktopparser.cpp
	item.cpp
	itemsdatabase.cpp
	itemsfinder.cpp
	itemtypes.cpp
	xdg.cpp
	)

foreach (SRC ${XDG_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/xdg/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-xdg${LC_LIBSUFFIX} SHARED
	${XDG_SRCS}
	)
target_link_libraries (leechcraft-util-xdg${LC_LIBSUFFIX}
	leechcraft-util-xpc${LC_LIBSUFFIX}
	)
set_property (TARGET leechcraft-util-xdg${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-xdg${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-xdg${LC_LIBSUFFIX} Concurrent Widgets)
