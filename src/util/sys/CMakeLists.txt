add_util_library (util-sys
	SOURCES
		encodingconverter.cpp
		mimedetector.cpp
		paths.cpp
		resourceloader.cpp
		sysinfo.cpp
		extensionsdata.cpp
		util.cpp
		fdguard.cpp
		cpufeatures.cpp
		timer.cpp
	DEPENDS
		leechcraft-util-sll
		leechcraft-util
	USES Network Widgets
	)
