AddCoreTest (tagsmanager CoreTagsManagerTest
	SRCS
		tagsmanagertest.cpp
		../tagsmanager.cpp
		../tagsstorage.cpp
	LINK_LIBRARIES
		leechcraft-util-db
		leechcraft-util-models
		leechcraft-util-sll
		leechcraft-util-sys
		leechcraft-util-tags
	)
