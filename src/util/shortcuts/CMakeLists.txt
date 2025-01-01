set (SHORTCUTS_SRCS
	shortcutmanager.cpp
	util.cpp
	)

add_util_library (util-shortcuts
	SOURCES ${SHORTCUTS_SRCS}
	DEPENDS
		leechcraft-util-xpc
		leechcraft-util-sll
	USES Widgets
	)
