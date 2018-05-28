include_directories (${CMAKE_CURRENT_BINARY_DIR})
set (DB_SRCS
	backendselector.cpp
	dblock.cpp
	util.cpp
	closingdb.cpp
	dumper.cpp
	consistencychecker.cpp
	)
set (DB_FORMS
	backendselector.ui
	)

qt5_wrap_ui (DB_UIS_H ${DB_FORMS})

foreach (SRC ${DB_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/db/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

foreach (FORM ${DB_FORMS})
	set (UTIL_FORMS "${UTIL_FORMS};util/db/${FORM}")
endforeach ()
set (UTIL_FORMS ${UTIL_FORMS} PARENT_SCOPE)

add_library (leechcraft-util-db${LC_LIBSUFFIX} SHARED
	${DB_SRCS}
	${DB_UIS_H}
	)
target_link_libraries (leechcraft-util-db${LC_LIBSUFFIX}
	leechcraft-xsd${LC_LIBSUFFIX}
	)
set_property (TARGET leechcraft-util-db${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION}.1)
install (TARGETS leechcraft-util-db${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-db${LC_LIBSUFFIX} Concurrent Sql Widgets)

if (ENABLE_UTIL_TESTS)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR})

	AddUtilTest (db_oral_sqlite tests/oraltest.cpp UtilDbOralTestSqlite leechcraft-util-db${LC_LIBSUFFIX})
	AddUtilTest (db_oralfkey_sqlite tests/oralfkeytest.cpp UtilDbOralFKeyTestSqlite leechcraft-util-db${LC_LIBSUFFIX})

	target_compile_definitions (lc_util_db_oral_sqlite_test PUBLIC -DORAL_FACTORY=ORAL_FACTORY_SQLITE)
	target_compile_definitions (lc_util_db_oralfkey_sqlite_test PUBLIC -DORAL_FACTORY=ORAL_FACTORY_SQLITE)

	AddUtilTest (db_oral_postgres tests/oraltest.cpp UtilDbOralTestPostgres leechcraft-util-db${LC_LIBSUFFIX})
	AddUtilTest (db_oralfkey_postgres tests/oralfkeytest.cpp UtilDbOralFKeyTestPostgres leechcraft-util-db${LC_LIBSUFFIX})

	target_compile_definitions (lc_util_db_oral_postgres_test PUBLIC -DORAL_FACTORY=ORAL_FACTORY_POSTGRES)
	target_compile_definitions (lc_util_db_oralfkey_postgres_test PUBLIC -DORAL_FACTORY=ORAL_FACTORY_POSTGRES)
endif ()
