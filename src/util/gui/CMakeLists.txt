include_directories (${CMAKE_CURRENT_BINARY_DIR})

set (IS_FREEDESKTOP_PLATFORM 0)
set (MAYBE_DBUS_DEP "")
# A generator expression (like with PLATFORM_ID) does not work in this case,
# since the list of libraries on FindQtLibs depends on its value,
# but the generator won't be evaluated soon enough.
set (FREEDESKTOP_PLATFORMS "FreeBSD;Linux;NetBSD;OpenBSD")
if ("${CMAKE_SYSTEM_NAME}" IN_LIST FREEDESKTOP_PLATFORMS)
	set (IS_FREEDESKTOP_PLATFORM 1)
	set (MAYBE_DBUS_DEP DBus)
endif ()

set (GUI_SRCS
	autoresizemixin.cpp
	clearlineeditaddon.cpp
	colorbutton.cpp
	fancytrayicon.cpp
	fancytrayiconfallback.cpp
	$<${IS_FREEDESKTOP_PLATFORM}:fancytrayiconfreedesktop.cpp>
	fancytrayiconimpl.cpp
	findnotification.cpp
	flowlayout.cpp
	geometry.cpp
	lineeditbuttonmanager.cpp
	pagenotification.cpp
	selectablebrowser.cpp
	sslcertificateinfowidget.cpp
	unhoverdeletemixin.cpp
	util.cpp
	fontsizescrollchanger.cpp
	statesaver.cpp
	menumodeladapter.cpp
	)
set (GUI_FORMS
	findnotification.ui
	sslcertificateinfowidget.ui
	)

add_compile_definitions ($<${IS_FREEDESKTOP_PLATFORM}:IS_FREEDESKTOP_PLATFORM>)

qt_wrap_ui (GUI_UIS_H ${GUI_FORMS})

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

FindQtLibs (leechcraft-util-gui${LC_LIBSUFFIX}
	${MAYBE_DBUS_DEP}
	Network
	Widgets
	)

if (ENABLE_UTIL_TESTS)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR})

	AddUtilTest (gui_util tests/utiltest.cpp UtilGuiUtilTest leechcraft-util-gui${LC_LIBSUFFIX})
endif ()
