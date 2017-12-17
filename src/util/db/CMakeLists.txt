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

	AddUtilTest (db_oral tests/oraltest.cpp UtilDbOralTest leechcraft-util-db${LC_LIBSUFFIX})
	AddUtilTest (db_oralfkey tests/oralfkeytest.cpp UtilDbOralFKeyTest leechcraft-util-db${LC_LIBSUFFIX})
endif ()
