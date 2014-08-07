set (XSD_SRCS
	addressesmodelmanager.cpp
	util.cpp
	)

foreach (SRC ${XSD_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/xsd/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-xsd SHARED
	${XSD_SRCS}
	)
target_link_libraries (leechcraft-util-xsd
	${QT_LIBRARIES}
	leechcraft-xsd
	leechcraft-util-network
	)
set_property (TARGET leechcraft-util-xsd PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-xsd DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-xsd Network Widgets)
