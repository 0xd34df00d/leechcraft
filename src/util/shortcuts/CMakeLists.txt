set (SHORTCUTS_SRCS
	shortcutmanager.cpp
	util.cpp
	)

foreach (SRC ${SHORTCUTS_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/shortcuts/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_util_library (util-shortcuts
	SOURCES ${SHORTCUTS_SRCS}
	DEPENDS
		leechcraft-util-xpc
		leechcraft-util-sll
	USES Widgets
	)
