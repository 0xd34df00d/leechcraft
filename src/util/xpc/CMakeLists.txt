set (XPC_SRCS
	basehookinterconnector.cpp
	defaulthookproxy.cpp
	notificationactionhandler.cpp
	passutils.cpp
	stdanfields.cpp
	stddatafiltermenucreator.cpp
	util.cpp
	)

foreach (SRC ${XPC_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/xpc/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-xpc SHARED
	${XPC_SRCS}
	)
target_link_libraries (leechcraft-util-xpc
	${QT_LIBRARIES}
	)
set_property (TARGET leechcraft-util-xpc PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-xpc DESTINATION ${LIBDIR})
