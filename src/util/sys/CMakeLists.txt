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

add_library (leechcraft-util-sys${LC_LIBSUFFIX} SHARED
	${SYS_SRCS}
	)
target_link_libraries (leechcraft-util-sys${LC_LIBSUFFIX}
	leechcraft-util-sll${LC_LIBSUFFIX}
	leechcraft-util${LC_LIBSUFFIX}
	)
set_property (TARGET leechcraft-util-sys${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION}.1)
install (TARGETS leechcraft-util-sys${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-sys${LC_LIBSUFFIX} Network Widgets)
