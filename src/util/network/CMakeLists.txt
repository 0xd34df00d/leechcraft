set (NETWORK_SRCS
	addresses.cpp
	customcookiejar.cpp
	customnetworkreply.cpp
	handlenetworkreply.cpp
	lcserviceoverride.cpp
	networkdiskcache.cpp
	networkdiskcachegc.cpp
	socketerrorstrings.cpp
	sslerror2treeitem.cpp
	)

foreach (SRC ${NETWORK_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/network/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_util_library (util-network
	SOURCES ${NETWORK_SRCS}
	DEPENDS
		leechcraft-util-sll
		leechcraft-util-sys
	USES Concurrent Network
	)
