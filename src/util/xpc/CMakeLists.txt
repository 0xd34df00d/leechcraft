set (XPC_SRCS
	basehookinterconnector.cpp
	defaulthookproxy.cpp
	notificationactionhandler.cpp
	passutils.cpp
	stdanfields.cpp
	stddatafiltermenucreator.cpp
	util.cpp
	anutil.cpp
	)

foreach (SRC ${XPC_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/xpc/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-xpc${LC_LIBSUFFIX} SHARED
	${XPC_SRCS}
	)
target_link_libraries (leechcraft-util-xpc${LC_LIBSUFFIX}
	leechcraft-util-sll${LC_LIBSUFFIX}
	)
set_property (TARGET leechcraft-util-xpc${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION}.1)
install (TARGETS leechcraft-util-xpc${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-xpc${LC_LIBSUFFIX} Widgets)
