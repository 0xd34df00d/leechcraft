add_util_library (util-sys
	SRCS
		encodingconverter.cpp
		mimedetector.cpp
		paths.cpp
		resourceloader.cpp
		sysinfo.cpp
		extensionsdata.cpp
		util.cpp
		fdguard.cpp
		cpufeatures.cpp
		loggingfilter.cpp
	DEPENDS
		leechcraft-util-sll
		leechcraft-util
	USES Network Widgets
	)
