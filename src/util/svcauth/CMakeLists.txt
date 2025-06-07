add_util_library (util-svcauth
	SRCS
		ljutils.cpp
		vkauthmanager.cpp
		vkcaptchadialog.cpp
	DEPENDS
		leechcraft-util-network
		leechcraft-util-sll
		leechcraft-util-threads
		leechcraft-util-xpc
	USES Network Widgets
	)
