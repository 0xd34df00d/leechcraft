add_util_library (util-db
	SRCS
		backendselector.cpp
		dblock.cpp
		util.cpp
		closingdb.cpp
		dumper.cpp
		consistencychecker.cpp
	DEPENDS
		leechcraft-util-gui
		leechcraft-util-sys
		leechcraft-util-threads
		leechcraft-util
		leechcraft-xsd
	USES Concurrent Sql Widgets
	)

option (ENABLE_UTIL_DB_ORAL_POSTGRES_TESTS "Enable Util db/oral tests of PostgreSQL backend layer" OFF)

if (ENABLE_UTIL_TESTS)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR})

	AddUtilTest (db_oral_sqlite tests/oraltest.cpp UtilDbOralTestSqlite leechcraft-util-db)
	AddUtilTest (db_oral_simplerecord_sqlite tests/oraltest_simplerecord.cpp UtilDbOralTestSimpleRecordSqlite leechcraft-util-db)
	AddUtilTest (db_oral_simplerecord_bench_sqlite tests/oraltest_simplerecord_bench.cpp UtilDbOralTestSimpleRecordBenchSqlite leechcraft-util-db)
	AddUtilTest (db_oralfkey_sqlite tests/oralfkeytest.cpp UtilDbOralFKeyTestSqlite leechcraft-util-db)

	target_compile_definitions (lc_util_db_oral_sqlite_test PUBLIC -DORAL_FACTORY=ORAL_FACTORY_SQLITE)
	target_compile_definitions (lc_util_db_oral_simplerecord_sqlite_test PUBLIC -DORAL_FACTORY=ORAL_FACTORY_SQLITE)
	target_compile_definitions (lc_util_db_oral_simplerecord_bench_sqlite_test PUBLIC -DORAL_FACTORY=ORAL_FACTORY_SQLITE)
	target_compile_definitions (lc_util_db_oralfkey_sqlite_test PUBLIC -DORAL_FACTORY=ORAL_FACTORY_SQLITE)

	if (ENABLE_UTIL_DB_ORAL_POSTGRES_TESTS)
		AddUtilTest (db_oral_postgres tests/oraltest.cpp UtilDbOralTestPostgres leechcraft-util-db)
		AddUtilTest (db_oral_simplerecord_postgres tests/oraltest_simplerecord.cpp UtilDbOralTestSimpleRecordPostgres leechcraft-util-db)
		AddUtilTest (db_oralfkey_postgres tests/oralfkeytest.cpp UtilDbOralFKeyTestPostgres leechcraft-util-db)

		target_compile_definitions (lc_util_db_oral_postgres_test PUBLIC -DORAL_FACTORY=ORAL_FACTORY_POSTGRES)
		target_compile_definitions (lc_util_db_oral_simplerecord_postgres_test PUBLIC -DORAL_FACTORY=ORAL_FACTORY_POSTGRES)
		target_compile_definitions (lc_util_db_oralfkey_postgres_test PUBLIC -DORAL_FACTORY=ORAL_FACTORY_POSTGRES)
	endif ()
endif ()
