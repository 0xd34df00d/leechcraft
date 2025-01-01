set (XDG_SRCS
	desktopparser.cpp
	item.cpp
	itemsdatabase.cpp
	itemsfinder.cpp
	itemtypes.cpp
	xdg.cpp
	)

add_util_library (util-xdg
	SOURCES ${XDG_SRCS}
	DEPENDS leechcraft-util-xpc
	USES Concurrent Widgets
	)

if (ENABLE_UTIL_TESTS)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR})

	AddUtilTest (xdg_desktopparser tests/desktopparsertest.cpp UtilXdgDesktopParserTest leechcraft-util-xdg)
endif ()
