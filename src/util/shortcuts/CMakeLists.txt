set (SHORTCUTS_SRCS
	shortcutmanager.cpp
	)

foreach (SRC ${SHORTCUTS_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/shortcuts/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-shortcuts SHARED
	${SHORTCUTS_SRCS}
	)
target_link_libraries (leechcraft-util-shortcuts
	leechcraft-util-xpc
	)
set_property (TARGET leechcraft-util-shortcuts PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-shortcuts DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-shortcuts Widgets)
