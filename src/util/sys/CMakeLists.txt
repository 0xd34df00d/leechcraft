set (SYS_SRCS
	mimedetector.cpp
	paths.cpp
	resourceloader.cpp
	sysinfo.cpp
	extensionsdata.cpp
	util.cpp
	fdguard.cpp
	cpufeatures.cpp
	timer.cpp
	)

add_util_library (util-sys
	SOURCES ${SYS_SRCS}
	DEPENDS
		leechcraft-util-sll
		leechcraft-util
	USES Network Widgets
	)
