set (SVCAUTH_SRCS
	vkauthmanager.cpp
	vkcaptchadialog.cpp
	)
set (SVCAUTH_FORMS
	vkcaptchadialog.ui
	)

qt_wrap_ui (SVCAUTH_UIS_H ${SVCAUTH_FORMS})

add_util_library (util-svcauth
	SOURCES ${SVCAUTH_SRCS} ${SVCAUTH_UIS_H}
	DEPENDS
		leechcraft-util-network
		leechcraft-util-sll
		leechcraft-util-xpc
	USES Network Widgets
	)
