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

add_util_library (util-gui
	SRCS
		actionresultreporter.cpp
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
		progressdelegate.cpp
	DEPENDS
		leechcraft-util-network
		leechcraft-util-shortcuts
		leechcraft-util-sll
		leechcraft-util-xpc
		leechcraft-xsd
	USES ${MAYBE_DBUS_DEP} Network Widgets
	DEFINES $<${IS_FREEDESKTOP_PLATFORM}:IS_FREEDESKTOP_PLATFORM>
	)

if (ENABLE_UTIL_TESTS)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR})
	AddUtilTest (gui_util tests/utiltest.cpp UtilGuiUtilTest leechcraft-util-gui)
endif ()
