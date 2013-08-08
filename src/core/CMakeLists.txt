INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	)

ADD_DEFINITIONS (-DBOOST_PROGRAM_OPTIONS_DYN_LINK)
ADD_DEFINITIONS (${QT_DEFINITIONS})

OPTION (WITH_DOCS "Enable building documentation (requires Doxygen)" OFF)
OPTION (WITH_DOCS_INSTALL "Install generated documentation (if WITH_DOCS is set)" OFF)
IF (WITH_DOCS)
	FIND_PACKAGE (Doxygen REQUIRED)

	SET (DOXYDIR "${CMAKE_CURRENT_SOURCE_DIR}/../../doc/doxygen/core/")
	SET (DOXYFILE "${DOXYDIR}/Doxyfile")

	ADD_CUSTOM_TARGET (doc_core ALL
		"sed" "-i" "s/^PROJECT_NUMBER.*/PROJECT_NUMBER = '${LEECHCRAFT_VERSION}'/" "${DOXYFILE}"
		COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYFILE}"
		COMMAND "mv" "-f" "${DOXYDIR}/out" "${CMAKE_CURRENT_BINARY_DIR}/out"
		WORKING_DIRECTORY "${DOXYDIR}"
	)
	IF (WITH_DOCS_INSTALL)
		INSTALL (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/out/html" DESTINATION "share/doc/leechcraft-core-${LEECHCRAFT_VERSION}")
	ENDIF (WITH_DOCS_INSTALL)
ENDIF (WITH_DOCS)

OPTION (WITH_X11 "Enable X11 support on Linux" ON)
IF (WITH_X11 AND NOT APPLE)
	FIND_PACKAGE (X11)

	IF (X11_FOUND)
		ADD_DEFINITIONS (-DHAVE_X11)
		SET (ADDITIONAL_LIBS ${X11_X11_LIB})
	ENDIF (X11_FOUND)
ENDIF (WITH_X11 AND NOT APPLE)

SET (SRCS
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
	authenticationdialog.cpp
	sslerrorsdialog.cpp
	storagebackend.cpp
	sqlstoragebackend.cpp
	debugmessagehandler.cpp
	application.cpp
	handlerchoicedialog.cpp
	shortcutmanager.cpp
	keysequencer.cpp
	networkdiskcache.cpp
	networkaccessmanager.cpp
	coreproxy.cpp
	tagsmanager.cpp
	tagsviewer.cpp
	aboutdialog.cpp
	startupwizard.cpp
	localsockethandler.cpp
	toolbarguard.cpp
	wizardtypechoicepage.cpp
	newtabmenumanager.cpp
	plugintreebuilder.cpp
	coreinstanceobject.cpp
	settingstab.cpp
	separatetabbar.cpp
	separatetabwidget.cpp
	mwproxy.cpp
	winwarndialog.cpp
	coreplugin2manager.cpp
	dockmanager.cpp${_currentArg}
	acceptlangwidget.cpp
	entitymanager.cpp
	colorthemeengine.cpp
	rootwindowsmanager.cpp
	docktoolbarmanager.cpp
	)
SET (FORMS
	commonjobadder.ui
	leechcraft.ui
	pluginmanagerdialog.ui
	authenticationdialog.ui
	sslerrorsdialog.ui
	handlerchoicedialog.ui
	shortcutmanager.ui
	keysequencer.ui
	tagsviewer.ui
	aboutdialog.ui
	wizardtypechoicepage.ui
	settingstab.ui
	acceptlangwidget.ui
	)
SET (RESOURCES mainresources.qrc)

CreateTrs ("" "ru_RU;de;uk_UA;it;ar;eo;es;tr;tt;en;be;fr;pl" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("" "en;ru_RU" "${SRCS};${UTIL_SRCS};${XSD_SRCS}" "${FORMS};${UTIL_FORMS};${XSD_FORMS}" "coresettings.xml")

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCC ${RESOURCES})

ADD_EXECUTABLE (leechcraft WIN32
	../config.h
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCC}
	)

FindQtLibs (leechcraft Gui Network Widgets Xml Sql Script Concurrent DBus Svg)

TARGET_LINK_LIBRARIES (leechcraft
		${QT_QTMAIN_LIBRARY}
		${QT_LIBRARIES}
		${Boost_PROGRAM_OPTIONS_LIBRARY}
		${LEECHCRAFT_LIBRARIES}
		${ADDITIONAL_LIBS}
		)
INSTALL (TARGETS leechcraft DESTINATION ${LC_BINDIR})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES coresettings.xml DESTINATION ${LC_SETTINGS_DEST})
