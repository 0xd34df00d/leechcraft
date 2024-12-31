cmake_policy (SET CMP0115 NEW)

include_directories (${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	)

option (WITH_DBUS_LOADERS "Build D-Bus loaders on UNIX" OFF)

if (WITH_DBUS_LOADERS)
	add_definitions (-DWITH_DBUS_LOADERS)
endif ()

option (WITH_QWT "Enable support for Qwt (for QML PlotItem, for example)" ON)
if (WITH_QWT)
	add_definitions (-DWITH_QWT)
endif ()

add_definitions (-DBOOST_PROGRAM_OPTIONS_DYN_LINK)

option (WITH_DOCS "Enable building documentation (requires Doxygen)" OFF)
option (WITH_DOCS_INSTALL "Install generated documentation (if WITH_DOCS is set)" OFF)
if (WITH_DOCS)
	find_package (Doxygen REQUIRED)

	set (DOXYDIR "${CMAKE_CURRENT_SOURCE_DIR}/../../doc/doxygen/core/")
	set (DOXYFILE "${DOXYDIR}/Doxyfile")

	add_custom_target (doc_core ALL
		"sed" "-i" "s/^PROJECT_NUMBER.*/PROJECT_NUMBER = '${LEECHCRAFT_VERSION}'/" "${DOXYFILE}"
		COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYFILE}"
		COMMAND "mv" "-f" "${DOXYDIR}/out" "${CMAKE_CURRENT_BINARY_DIR}/out"
		WORKING_DIRECTORY "${DOXYDIR}"
	)
	if (WITH_DOCS_INSTALL)
		install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/out/html" DESTINATION "share/doc/leechcraft-core-${LEECHCRAFT_VERSION}")
	endif ()
endif ()

option (WITH_X11 "Enable X11 support on Linux" ON)
if (WITH_X11 AND NOT APPLE)
	find_package (X11)

	if (X11_FOUND)
		add_definitions (-DHAVE_X11)
		set (ADDITIONAL_LIBS ${X11_X11_LIB})
	endif ()
endif ()

set (SRCS
	components/network/cookiesaver.cpp
	components/network/networkaccessmanager.cpp
	mainwindow.cpp
	commonjobadder.cpp
	core.cpp
	main.cpp
	pluginmanager.cpp
	xmlsettingsmanager.cpp
	pluginmanagerdialog.cpp
	iconthemeengine.cpp
	childactioneventfilter.cpp
	tabmanager.cpp
	sslerrorsdialog.cpp
	debugmessagehandler.cpp
	application.cpp
	handlerchoicedialog.cpp
	shortcutmanager.cpp
	keysequencer.cpp
	coreproxy.cpp
	tagsmanager.cpp
	tagsviewer.cpp
	aboutdialog.cpp
	startupwizard.cpp
	localsockethandler.cpp
	wizardtypechoicepage.cpp
	newtabmenumanager.cpp
	plugintreebuilder.cpp
	coreinstanceobject.cpp
	settingstab.cpp
	settingswidget.cpp
	separatetabbar.cpp
	separatetabwidget.cpp
	mwproxy.cpp
	winwarndialog.cpp
	coreplugin2manager.cpp
	dockmanager.cpp
	entitymanager.cpp
	colorthemeengine.cpp
	rootwindowsmanager.cpp
	docktoolbarmanager.cpp
	mainwindowmenumanager.cpp
	sslerrorshandler.cpp
	loadprocess.cpp
	loadprocessbase.cpp
	loadprogressreporter.cpp
	splashscreen.cpp
	tagsstorage.cpp
	clargs.cpp
	loaders/ipluginloader.cpp
	loaders/sopluginloader.cpp
	)

if (WITH_DBUS_LOADERS)
	set (SRCS
		${SRCS}
		loaders/dbuspluginloader.cpp
		loaders/infoproxy.cpp
		loaders/dbus/marshalling.cpp
		loaders/dbus/coreproxyserverwrapper.cpp
		loaders/dbus/coreproxyproxy.cpp
	)
endif ()

set (FORMS
	commonjobadder.ui
	leechcraft.ui
	pluginmanagerdialog.ui
	sslerrorsdialog.ui
	handlerchoicedialog.ui
	shortcutmanager.ui
	keysequencer.ui
	tagsviewer.ui
	aboutdialog.ui
	wizardtypechoicepage.ui
	settingstab.ui
	settingswidget.ui
	)
set (RESOURCES mainresources.qrc)

set (UTILITY_SRCS)
foreach (ITEM IN LISTS UTIL_SRCS XSD_SRCS)
	list (APPEND UTILITY_SRCS "../${ITEM}")
endforeach ()

set (UTILITY_FORMS)
foreach (ITEM IN LISTS UTIL_FORMS XSD_FORMS)
	list (APPEND UTILITY_FORMS "../${ITEM}")
endforeach ()

CreateTrs (COMPILED_TRANSLATIONS)

qt_wrap_ui (UIS_H ${FORMS})
qt_add_resources (RCC ${RESOURCES})

set (WIN_RESOURCES)
if (WIN32)
	set (WIN_RESOURCES ../../../tools/win32/leechcraft.rc)
endif ()

add_executable (leechcraft-core WIN32
	../config.h
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCC}
	${WIN_RESOURCES}
	)
if (NOT APPLE)
	set_target_properties (leechcraft-core PROPERTIES
		OUTPUT_NAME leechcraft${LC_LIBSUFFIX}
		)
else ()
	set_target_properties (leechcraft-core PROPERTIES
		OUTPUT_NAME leechcraft
		)
endif ()

target_link_libraries (leechcraft-core
		${Boost_PROGRAM_OPTIONS_LIBRARY}
		${LEECHCRAFT_LIBRARIES}
		${ADDITIONAL_LIBS}
		)

if (APPLE AND USE_UNIX_LAYOUT)
	get_property (_core_link_flags TARGET leechcraft-core PROPERTY LINK_FLAGS)
	set (_core_link_flags "${_core_link_flags} -sectcreate __TEXT __info_plist ${CMAKE_CURRENT_BINARY_DIR}/../Info.plist")
	message (STATUS "Setting Core link flags to: ${_core_link_flags}")
	set_target_properties (leechcraft-core PROPERTIES LINK_FLAGS ${_core_link_flags})
endif ()

install (TARGETS leechcraft-core DESTINATION ${LC_BINDIR})
install (FILES coresettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft-core Gui Network Widgets Xml Quick Qml Concurrent Svg)

if (WITH_DBUS_LOADERS)
	add_subdirectory (loaders/dbus)
	FindQtLibs (leechcraft-core DBus)
endif ()
