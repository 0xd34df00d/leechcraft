set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

find_package (Boost REQUIRED COMPONENTS filesystem)

set (QT_USE_QTDBUS TRUE)
include (${QT_USE_FILE})
include_directories (${Boost_INCLUDE_DIR})

set (SRCS
	udisks2backend.cpp
	dbus/manager.cpp
	)

set (UDISKS_BACKEND_LIB leechcraft_vrooby_udisks2)
set (UDISKS_BACKEND_LIBS "${UDISKS_BACKEND_LIBS};${UDISKS_BACKEND_LIB}" PARENT_SCOPE)

add_library (${UDISKS_BACKEND_LIB} STATIC
	${SRCS}
	)
target_link_libraries (${UDISKS_BACKEND_LIB}
	${QT_LIBRARIES}
	${Boost_FILESYSTEM_LIBRARY}
	)
