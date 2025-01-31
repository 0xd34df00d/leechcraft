add_util_library (util-shortcuts
	SRCS
		shortcutmanager.cpp
		util.cpp
	DEPENDS
		leechcraft-util-xpc
		leechcraft-util-sll
	USES Widgets
	)
