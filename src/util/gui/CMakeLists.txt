include_directories (${CMAKE_CURRENT_BINARY_DIR})
set (GUI_SRCS
	autoresizemixin.cpp
	clearlineeditaddon.cpp
	colorbutton.cpp
	findnotification.cpp
	findnotificationwk.cpp
	flowlayout.cpp
	lineeditbuttonmanager.cpp
	pagenotification.cpp
	selectablebrowser.cpp
	sslcertificateinfowidget.cpp
	unhoverdeletemixin.cpp
	util.cpp
	)
set (GUI_FORMS
	findnotification.ui
	sslcertificateinfowidget.ui
	)

qt5_wrap_ui (GUI_UIS_H ${GUI_FORMS})

foreach (SRC ${GUI_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/gui/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

foreach (FORM ${GUI_FORMS})
	set (UTIL_FORMS "${UTIL_FORMS};util/gui/${FORM}")
endforeach ()
set (UTIL_FORMS ${UTIL_FORMS} PARENT_SCOPE)

add_library (leechcraft-util-gui${LC_LIBSUFFIX} SHARED
	${GUI_SRCS}
	${GUI_UIS_H}
	)
target_link_libraries (leechcraft-util-gui${LC_LIBSUFFIX}
	leechcraft-util-network${LC_LIBSUFFIX}
	leechcraft-util-shortcuts${LC_LIBSUFFIX}
	leechcraft-util-sll${LC_LIBSUFFIX}
	)
set_property (TARGET leechcraft-util-gui${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION}.1)
install (TARGETS leechcraft-util-gui${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-gui${LC_LIBSUFFIX} Network WebKitWidgets Widgets)
