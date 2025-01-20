add_util_library (util-svcauth
	SOURCES
		ljutils.cpp
		vkauthmanager.cpp
		vkcaptchadialog.cpp
	DEPENDS
		leechcraft-util-network
		leechcraft-util-sll
		leechcraft-util-xpc
	USES Network Widgets
	)
