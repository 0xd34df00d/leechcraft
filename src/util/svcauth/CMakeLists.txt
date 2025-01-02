set (SVCAUTH_SRCS
	vkauthmanager.cpp
	vkcaptchadialog.cpp
	)

add_util_library (util-svcauth
	SOURCES ${SVCAUTH_SRCS}
	DEPENDS
		leechcraft-util-network
		leechcraft-util-sll
		leechcraft-util-xpc
	USES Network Widgets
	)
