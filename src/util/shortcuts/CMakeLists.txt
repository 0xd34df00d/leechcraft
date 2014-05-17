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
	${QT_LIBRARIES}
	leechcraft-util-xpc
	)
set_property (TARGET leechcraft-util-shortcuts PROPERTY SOVERSION 0.6.65)
install (TARGETS leechcraft-util-shortcuts DESTINATION ${LIBDIR})
