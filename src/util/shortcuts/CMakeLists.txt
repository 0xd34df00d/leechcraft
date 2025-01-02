add_util_library (util-shortcuts
	SOURCES
		shortcutmanager.cpp
		util.cpp
	DEPENDS
		leechcraft-util-xpc
		leechcraft-util-sll
	USES Widgets
	)
