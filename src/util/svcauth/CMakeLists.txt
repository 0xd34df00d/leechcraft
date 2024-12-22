include_directories (${CMAKE_CURRENT_BINARY_DIR})
set (SVCAUTH_SRCS
	vkauthmanager.cpp
	vkcaptchadialog.cpp
	)
set (SVCAUTH_FORMS
	vkcaptchadialog.ui
	)

qt_wrap_ui (SVCAUTH_UIS_H ${SVCAUTH_FORMS})

foreach (SRC ${SVCAUTH_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/svcauth/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

foreach (FORM ${SVCAUTH_FORMS})
	set (UTIL_FORMS "${UTIL_FORMS};util/svcauth/${FORM}")
endforeach ()
set (UTIL_FORMS ${UTIL_FORMS} PARENT_SCOPE)

add_util_library (util-svcauth
	SOURCES ${SVCAUTH_SRCS} ${SVCAUTH_UIS_H}
	DEPENDS
		leechcraft-util-network
		leechcraft-util-sll
		leechcraft-util-xpc
	USES Network Widgets
	)
