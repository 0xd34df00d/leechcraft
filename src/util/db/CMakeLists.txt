add_util_library (util-db
	SRCS
		dblock.cpp
		util.cpp
		closingdb.cpp
		dumper.cpp
		consistencychecker.cpp
	DEPENDS
		leechcraft-util-gui
		leechcraft-util-sll
		leechcraft-util-sys
		leechcraft-util-threads
		leechcraft-util
		leechcraft-xsd
	USES Concurrent Sql Widgets
	)

if (ENABLE_UTIL_TESTS)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR})

	AddUtilTest (db_oral_sqlite tests/oraltest.cpp UtilDbOralTestSqlite leechcraft-util-db)
	AddUtilTest (db_oral_simplerecord_sqlite tests/oraltest_simplerecord.cpp UtilDbOralTestSimpleRecordSqlite leechcraft-util-db)
	AddUtilTest (db_oral_simplerecord_bench_sqlite tests/oraltest_simplerecord_bench.cpp UtilDbOralTestSimpleRecordBenchSqlite leechcraft-util-db)
	AddUtilTest (db_oralfkey_sqlite tests/oralfkeytest.cpp UtilDbOralFKeyTestSqlite leechcraft-util-db)
endif ()
