set (X11_SRCS
	xwrapper.cpp
	)

find_package (X11)

if (X11_FOUND)
	add_definitions (-DHAVE_X11)
else ()
	message (CRITICAL "Don't try building libleechcraft-util-x11 if you don't have X11!")
endif ()

foreach (SRC ${X11_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/x11/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-x11 SHARED
	${X11_SRCS}
	)
target_link_libraries (leechcraft-util-x11
	${QT_LIBRARIES}
	${X11_X11_LIB}
	)
set_property (TARGET leechcraft-util-x11 PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-x11 DESTINATION ${LIBDIR})
