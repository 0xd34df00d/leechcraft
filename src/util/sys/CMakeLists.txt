find_package (MAGIC)
if (MAGIC_FOUND)
	add_definitions (-DHAVE_MAGIC)
else ()
	set (MAGIC_INCLUDE_DIRS)
	set (MAGIC_LIBRARIES)
endif ()

find_package (Boost REQUIRED COMPONENTS filesystem system)

include_directories (${MAGIC_INCLUDE_DIRS})

set (SYS_SRCS
	mimedetector.cpp
	paths.cpp
	resourceloader.cpp
	sysinfo.cpp
	extensionsdata.cpp
	util.cpp
	fdguard.cpp
	cpufeatures.cpp
	)

if (UNIX AND NOT APPLE)
	set (SYS_SRCS ${SYS_SRCS} extensionsdataimpl_x11.cpp)

	add_definitions (-DHAVE_EXTENSIONS_DATA)
elseif (APPLE)
	set (SYS_SRCS ${SYS_SRCS} extensionsdataimpl_mac.mm)

	add_definitions (-DHAVE_EXTENSIONS_DATA)
elseif (WIN32)
	set (SYS_SRCS ${SYS_SRCS} extensionsdataimpl_w32.cpp)

	add_definitions (-DHAVE_EXTENSIONS_DATA)
endif ()

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
	${MAGIC_LIBRARIES}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	)
set_property (TARGET leechcraft-util-sys${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION}.1)
install (TARGETS leechcraft-util-sys${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-sys${LC_LIBSUFFIX} Network Widgets)
