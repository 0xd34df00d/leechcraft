include_directories (${CMAKE_CURRENT_BINARY_DIR})
set (SVCAUTH_SRCS
	vkauthmanager.cpp
	vkcaptchadialog.cpp
	)
set (SVCAUTH_FORMS
	vkcaptchadialog.ui
	)

if (USE_QT5)
	qt5_wrap_ui (SVCAUTH_UIS_H ${SVCAUTH_FORMS})
else ()
	QT4_WRAP_UI (SVCAUTH_UIS_H ${SVCAUTH_FORMS})
endif ()

foreach (SRC ${SVCAUTH_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/svcauth/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

foreach (FORM ${SVCAUTH_FORMS})
	set (UTIL_FORMS "${UTIL_FORMS};util/svcauth/${FORM}")
endforeach ()
set (UTIL_FORMS ${UTIL_FORMS} PARENT_SCOPE)

add_library (leechcraft-util-svcauth SHARED
	${SVCAUTH_SRCS}
	${SVCAUTH_UIS_H}
	)
target_link_libraries (leechcraft-util-svcauth
	${QT_LIBRARIES}
	leechcraft-util-network
	)
set_property (TARGET leechcraft-util-svcauth PROPERTY SOVERSION 0.6.65)
install (TARGETS leechcraft-util-svcauth DESTINATION ${LIBDIR})

if (USE_QT5)
	QT5_USE_MODULES (leechcraft-util-svcauth Network Widgets)
endif ()
