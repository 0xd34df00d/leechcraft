set (THREADS_SRCS
	workerthreadbase.cpp
	)

foreach (SRC ${THREADS_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/threads/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-threads${LC_LIBSUFFIX} SHARED
	${THREADS_SRCS}
	)
target_link_libraries (leechcraft-util-threads${LC_LIBSUFFIX}
	leechcraft-util-sll${LC_LIBSUFFIX}
	)
set_property (TARGET leechcraft-util-threads${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-threads${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-threads${LC_LIBSUFFIX} Core Concurrent)

if (ENABLE_UTIL_TESTS)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR})
	AddUtilTest (threads_futures tests/futures.cpp UtilThreadsFuturesTest leechcraft-util-sll${LC_LIBSUFFIX})
endif ()
