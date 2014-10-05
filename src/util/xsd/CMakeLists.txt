set (XSD_SRCS
	addressesmodelmanager.cpp
	util.cpp
	)

foreach (SRC ${XSD_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/xsd/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-xsd${LC_LIBSUFFIX} SHARED
	${XSD_SRCS}
	)
target_link_libraries (leechcraft-util-xsd${LC_LIBSUFFIX}
	leechcraft-xsd${LC_LIBSUFFIX}
	leechcraft-util-network${LC_LIBSUFFIX}
	)
set_property (TARGET leechcraft-util-xsd${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-xsd${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-xsd${LC_LIBSUFFIX} Network Widgets)
