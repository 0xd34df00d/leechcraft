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

add_util_library (util-network
	SOURCES ${NETWORK_SRCS}
	DEPENDS
		leechcraft-util-sll
		leechcraft-util-sys
	USES Concurrent Network
	)
