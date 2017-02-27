include_directories (${CMAKE_CURRENT_BINARY_DIR})
set (SVCAUTH_SRCS
	vkauthmanager.cpp
	vkcaptchadialog.cpp
	)
set (SVCAUTH_FORMS
	vkcaptchadialog.ui
	)

qt5_wrap_ui (SVCAUTH_UIS_H ${SVCAUTH_FORMS})

foreach (SRC ${SVCAUTH_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/svcauth/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

foreach (FORM ${SVCAUTH_FORMS})
	set (UTIL_FORMS "${UTIL_FORMS};util/svcauth/${FORM}")
endforeach ()
set (UTIL_FORMS ${UTIL_FORMS} PARENT_SCOPE)

add_library (leechcraft-util-svcauth${LC_LIBSUFFIX} SHARED
	${SVCAUTH_SRCS}
	${SVCAUTH_UIS_H}
	)
target_link_libraries (leechcraft-util-svcauth${LC_LIBSUFFIX}
	leechcraft-util-network${LC_LIBSUFFIX}
	leechcraft-util-sll${LC_LIBSUFFIX}
	leechcraft-util-xpc${LC_LIBSUFFIX}
	)
set_property (TARGET leechcraft-util-svcauth${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-svcauth${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-svcauth${LC_LIBSUFFIX} Network WebKitWidgets Widgets)
