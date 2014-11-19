set (SHORTCUTS_SRCS
	shortcutmanager.cpp
	util.cpp
	)

foreach (SRC ${SHORTCUTS_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/shortcuts/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-shortcuts${LC_LIBSUFFIX} SHARED
	${SHORTCUTS_SRCS}
	)
target_link_libraries (leechcraft-util-shortcuts${LC_LIBSUFFIX}
	leechcraft-util-xpc${LC_LIBSUFFIX}
	)
set_property (TARGET leechcraft-util-shortcuts${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-shortcuts${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-shortcuts${LC_LIBSUFFIX} Widgets)
