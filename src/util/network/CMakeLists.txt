set (NETWORK_SRCS
	addresses.cpp
	customcookiejar.cpp
	customnetworkreply.cpp
	networkdiskcache.cpp
	networkdiskcachegc.cpp
	socketerrorstrings.cpp
	)

foreach (SRC ${NETWORK_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/network/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-network${LC_LIBSUFFIX} SHARED
	${NETWORK_SRCS}
	)
target_link_libraries (leechcraft-util-network${LC_LIBSUFFIX}
	leechcraft-util-sys${LC_LIBSUFFIX}
	)
set_property (TARGET leechcraft-util-network${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-network${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-network${LC_LIBSUFFIX} Concurrent Network)
