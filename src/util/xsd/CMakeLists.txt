include_directories (${CMAKE_CURRENT_BINARY_DIR})
set (XSD_SRCS
	addressesmodelmanager.cpp
	wkfontswidget.cpp
	util.cpp
	)
set (XSD_FORMS
	)

if (USE_QT5)
	qt5_wrap_ui (XSD_UIS_H ${XSD_FORMS})
else ()
	QT4_WRAP_UI (XSD_UIS_H ${XSD_FORMS})
endif ()

foreach (SRC ${XSD_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/xsd/${SRC}")
endforeach ()
foreach (FORM ${XSD_FORMS})
	set (UTIL_FORMS "${UTIL_FORMS};util/xsd/${FORM}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-xsd${LC_LIBSUFFIX} SHARED
	${XSD_SRCS}
	${XSD_UIS_H}
	)
target_link_libraries (leechcraft-util-xsd${LC_LIBSUFFIX}
	leechcraft-xsd${LC_LIBSUFFIX}
	leechcraft-util-network${LC_LIBSUFFIX}
	)
set_property (TARGET leechcraft-util-xsd${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-xsd${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-xsd${LC_LIBSUFFIX} Network Widgets)
