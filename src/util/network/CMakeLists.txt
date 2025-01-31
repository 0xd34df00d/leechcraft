add_util_library (util-network
	SRCS
		addresses.cpp
		customcookiejar.cpp
		customnetworkreply.cpp
		handlenetworkreply.cpp
		lcserviceoverride.cpp
		networkdiskcache.cpp
		networkdiskcachegc.cpp
		socketerrorstrings.cpp
		sslerror2treeitem.cpp
	DEPENDS
		leechcraft-util-sll
		leechcraft-util-sys
	USES Concurrent Network
	)
