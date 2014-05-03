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

add_library (leechcraft-util-xdg SHARED
	${XDG_SRCS}
	)
target_link_libraries (leechcraft-util-xdg
	${QT_LIBRARIES}
	${MAGIC_LIBRARIES}
	)
set_property (TARGET leechcraft-util-xdg PROPERTY SOVERSION 0.6.65)
install (TARGETS leechcraft-util-xdg DESTINATION ${LIBDIR})
