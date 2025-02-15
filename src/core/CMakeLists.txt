cmake_policy (SET CMP0115 NEW)

include_directories (${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	)

option (WITH_DBUS_LOADERS "Build D-Bus loaders on UNIX" OFF)
option (WITH_QWT "Enable support for Qwt (for QML PlotItem, for example)" ON)
option (WITH_X11 "Enable X11 support on Linux" ON)
if (WITH_X11 AND NOT APPLE)
	find_package (X11)
endif ()

add_compile_definitions (BOOST_PROGRAM_OPTIONS_DYN_LINK
	$<$<BOOL:${WITH_DBUS_LOADERS}>:WITH_DBUS_LOADERS>
	$<$<BOOL:${WITH_QWT}>:WITH_QWT>
	$<$<BOOL:${X11_FOUND}>:HAVE_X11>
	)

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

set (CMAKE_AUTORCC TRUE)

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
	$<$<BOOL:${WITH_DBUS_LOADERS}>:
		loaders/dbuspluginloader.cpp
		loaders/infoproxy.cpp
		loaders/dbus/marshalling.cpp
		loaders/dbus/coreproxyserverwrapper.cpp
		loaders/dbus/coreproxyproxy.cpp
		>
	)

set (RESOURCES
	mainresources.qrc
	$<$<PLATFORM_ID:Windows>:../../../tools/win32/leechcraft.rc>
	)

CreateTrs (COMPILED_TRANSLATIONS)

add_executable (leechcraft-core WIN32
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RESOURCES}
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
		$<$<BOOL:${X11_FOUND}>:${X11_X11_LIB}>
		stdc++exp
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

if (UNIX AND NOT APPLE)
	configure_file (freedesktop/leechcraft.desktop.in leechcraft${LC_LIBSUFFIX}.desktop @ONLY)
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/leechcraft${LC_LIBSUFFIX}.desktop DESTINATION share/applications)
	install (DIRECTORY freedesktop/icons/ DESTINATION share/icons/hicolor)

	configure_file (scripts/leechcraft-add-file.in leechcraft${LC_LIBSUFFIX}-add-file @ONLY)
	configure_file (scripts/leechcraft-handle-file.in leechcraft${LC_LIBSUFFIX}-handle-file @ONLY)
	install (PROGRAMS
		${CMAKE_CURRENT_BINARY_DIR}/leechcraft${LC_LIBSUFFIX}-add-file
		${CMAKE_CURRENT_BINARY_DIR}/leechcraft${LC_LIBSUFFIX}-handle-file
		DESTINATION ${LC_BINDIR}
		)

	configure_file (data/leechcraft.1.in leechcraft${LC_LIBSUFFIX}.1 @ONLY)
	configure_file (data/leechcraft-add-file.1.in leechcraft${LC_LIBSUFFIX}-add-file.1 @ONLY)
	configure_file (data/leechcraft-handle-file.1.in leechcraft${LC_LIBSUFFIX}-handle-file.1 @ONLY)
	set (MAN_FILES leechcraft${LC_LIBSUFFIX}.1 leechcraft${LC_LIBSUFFIX}-add-file.1 leechcraft${LC_LIBSUFFIX}-handle-file.1)
	if (NOT SKIP_MAN_COMPRESS)
		list (TRANSFORM MAN_FILES APPEND .gz OUTPUT_VARIABLE MAN_FILES_GZ)
		list (TRANSFORM MAN_FILES_GZ PREPEND ${CMAKE_CURRENT_BINARY_DIR}/ OUTPUT_VARIABLE ABSOLUTE_MAN_FILES_GZ)
		add_custom_target (data_leechcraft_gz ALL
			gzip -kf9 *.1
			DEPENDS ${MAN_FILES}
			BYPRODUCTS ${MAN_FILES_GZ}
			)

		install (FILES ${ABSOLUTE_MAN_FILES_GZ} DESTINATION share/man/man1)
	else ()
		list (TRANSFORM MAN_FILES PREPEND ${CMAKE_CURRENT_BINARY_DIR}/ OUTPUT_VARIABLE ABSOLUTE_MAN_FILES)
		install (FILES ${ABSOLUTE_MAN_FILES} DESTINATION share/man/man1)
	endif ()
endif ()
