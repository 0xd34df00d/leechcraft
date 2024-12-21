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

foreach (SRC ${SYS_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/sys/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_util_library (util-sys
	SOURCES ${SYS_SRCS}
	DEPENDS
		leechcraft-util-sll
		leechcraft-util
	USES Network Widgets
	)
