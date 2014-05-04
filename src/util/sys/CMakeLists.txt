find_package (MAGIC)
if (MAGIC_FOUND)
	add_definitions (-DHAVE_MAGIC)
else ()
	set (MAGIC_INCLUDE_DIRS)
	set (MAGIC_LIBRARIES)
endif ()

include_directories (${MAGIC_INCLUDE_DIRS})

set (SYS_SRCS
	fileremoveguard.cpp
	mimedetector.cpp
	paths.cpp
	resourceloader.cpp
	sysinfo.cpp
	)

foreach (SRC ${SYS_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/sys/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-sys SHARED
	${SYS_SRCS}
	)
target_link_libraries (leechcraft-util-sys
	${QT_LIBRARIES}
	${MAGIC_LIBRARIES}
	)
set_property (TARGET leechcraft-util-sys PROPERTY SOVERSION 0.6.65)
install (TARGETS leechcraft-util-sys DESTINATION ${LIBDIR})
