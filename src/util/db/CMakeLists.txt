include_directories (${CMAKE_CURRENT_BINARY_DIR})
set (DB_SRCS
	backendselector.cpp
	dblock.cpp
	)
set (DB_FORMS
	backendselector.ui
	)

if (USE_QT5)
	qt5_wrap_ui (DB_UIS_H ${DB_FORMS})
else ()
	QT4_WRAP_UI (DB_UIS_H ${DB_FORMS})
endif ()

foreach (SRC ${DB_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/db/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

foreach (FORM ${DB_FORMS})
	set (UTIL_FORMS "${UTIL_FORMS};util/db/${FORM}")
endforeach ()
set (UTIL_FORMS ${UTIL_FORMS} PARENT_SCOPE)

add_library (leechcraft-util-db SHARED
	${DB_SRCS}
	${DB_UIS_H}
	)
target_link_libraries (leechcraft-util-db
	${QT_LIBRARIES}
	leechcraft-xsd
	)
set_property (TARGET leechcraft-util-db PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-db DESTINATION ${LIBDIR})

if (USE_QT5)
	QT5_USE_MODULES (leechcraft-util-db Sql Widgets)
endif ()
