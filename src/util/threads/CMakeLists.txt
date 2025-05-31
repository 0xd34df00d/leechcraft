add_util_library (util-threads
	SRCS
		coro/asdomdocument.cpp
		coro/context.cpp
		coro/networkreply.cpp
		coro/networkresult.cpp
		coro/process.cpp
		coro/timer.cpp
		coro/throttle.cpp
		futures.cpp
		workerthreadbase.cpp
	DEPENDS leechcraft-util-sll
	USES Concurrent Network
	)

if (ENABLE_UTIL_TESTS)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR})
	AddUtilTest (threads_corotask tests/corotasktest.cpp UtilThreadsCoroTaskTest leechcraft-util-threads)
	AddUtilTest (threads_futures tests/futurestest.cpp UtilThreadsFuturesTest leechcraft-util-threads)
	AddUtilTest (threads_workerthread tests/workerthreadtest.cpp UtilThreadsWorkerThreadTest leechcraft-util-threads)
endif ()
