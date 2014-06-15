set (NETWORK_SRCS
	addresses.cpp
	customcookiejar.cpp
	customnetworkreply.cpp
	networkdiskcache.cpp
	socketerrorstrings.cpp
	)

foreach (SRC ${NETWORK_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/network/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-network SHARED
	${NETWORK_SRCS}
	)
target_link_libraries (leechcraft-util-network
	${QT_LIBRARIES}
	leechcraft-util-sys
	)
set_property (TARGET leechcraft-util-network PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-network DESTINATION ${LIBDIR})

if (USE_QT5)
	QT5_USE_MODULES (leechcraft-util-network Network)
endif ()
