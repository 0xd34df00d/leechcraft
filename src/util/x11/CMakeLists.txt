set (X11_SRCS
	xwrapper.cpp
	)

find_package (X11 REQUIRED)

add_definitions (-DHAVE_X11)

foreach (SRC ${X11_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/x11/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

set (EXTRA_LIBS)
if (USE_QT5)
	find_package (XCB COMPONENTS XCB REQUIRED)
	set (EXTRA_LIBS ${EXTRA_LIBS} XCB::XCB)
endif ()

add_library (leechcraft-util-x11${LC_LIBSUFFIX} SHARED
	${X11_SRCS}
	)
target_link_libraries (leechcraft-util-x11${LC_LIBSUFFIX}
	${X11_X11_LIB}
	${EXTRA_LIBS}
	)
set_property (TARGET leechcraft-util-x11${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-x11${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-x11${LC_LIBSUFFIX} X11Extras Widgets)
