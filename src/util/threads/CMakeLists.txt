set (THREADS_SRCS
	coro/context.cpp
	coro/networkreply.cpp
	coro/networkresult.cpp
	coro/timer.cpp
	futures.cpp
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

FindQtLibs (leechcraft-util-threads${LC_LIBSUFFIX} Core Concurrent Network)

if (ENABLE_UTIL_TESTS)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR})
	AddUtilTest (threads_corotask tests/corotasktest.cpp UtilThreadsCoroTaskTest leechcraft-util-threads${LC_LIBSUFFIX})
	AddUtilTest (threads_futures tests/futurestest.cpp UtilThreadsFuturesTest leechcraft-util-threads${LC_LIBSUFFIX})
	AddUtilTest (threads_monadicfuture tests/monadicfuturetest.cpp UtilThreadsMonadicFutureTest leechcraft-util-threads${LC_LIBSUFFIX})
	AddUtilTest (threads_workerthread tests/workerthreadtest.cpp UtilThreadsWorkerThreadTest leechcraft-util-threads${LC_LIBSUFFIX})
endif ()
