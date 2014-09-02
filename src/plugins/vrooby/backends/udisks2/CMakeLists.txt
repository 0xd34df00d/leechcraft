set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

find_package (Boost REQUIRED COMPONENTS filesystem)

include_directories (${Boost_INCLUDE_DIR})

set (BACKEND_SRCS
	udisks2backend.cpp
	dbus/manager.cpp
	)

set (UDISKS_BACKEND_LIB leechcraft_vrooby_udisks2)
set (UDISKS_BACKEND_LIBS "${UDISKS_BACKEND_LIBS};${UDISKS_BACKEND_LIB}" PARENT_SCOPE)

add_library (${UDISKS_BACKEND_LIB} STATIC
	${BACKEND_SRCS}
	)
target_link_libraries (${UDISKS_BACKEND_LIB}
	${Boost_FILESYSTEM_LIBRARY}
	)

FindQtLibs (${UDISKS_BACKEND_LIB} DBus Gui)
