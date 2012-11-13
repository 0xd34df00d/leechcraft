INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR})

IF (NOT USE_QT5)
	SET (QT_USE_QTNETWORK TRUE)
	SET (QT_USE_QTXML TRUE)
	SET (QT_USE_QTSQL TRUE)
	SET (QT_USE_QTSCRIPT TRUE)
	INCLUDE (${QT_USE_FILE})
ENDIF ()

ADD_DEFINITIONS (-DBOOST_PROGRAM_OPTIONS_DYN_LINK)
ADD_DEFINITIONS (${QT_DEFINITIONS})

SET (SRCS
	mainwindow.cpp
	commonjobadder.cpp
	core.cpp
	main.cpp
	pluginmanager.cpp
	xmlsettingsmanager.cpp
	pluginmanagerdialog.cpp
	skinengine.cpp
	childactioneventfilter.cpp
	tabmanager.cpp
	authenticationdialog.cpp
	sslerrorsdialog.cpp
	storagebackend.cpp
	sqlstoragebackend.cpp
	debugmessagehandler.cpp
	application.cpp
	handlerchoicedialog.cpp
	graphwidget.cpp
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
	dockmanager.cpp
	acceptlangwidget.cpp
	entitymanager.cpp
	)
SET (HEADERS
	mainwindow.h
	commonjobadder.h
	core.h
	pluginmanager.h
	xmlsettingsmanager.h
	pluginmanagerdialog.h
	skinengine.h
	childactioneventfilter.h
	tabmanager.h
	authenticationdialog.h
	sslerrorsdialog.h
	storagebackend.h
	sqlstoragebackend.h
	debugmessagehandler.h
	application.h
	handlerchoicedialog.h
	graphwidget.h
	shortcutmanager.h
	keysequencer.h
	networkdiskcache.h
	networkaccessmanager.h
	coreproxy.h
	tagsmanager.h
	tagsviewer.h
	aboutdialog.h
	startupwizard.h
	localsockethandler.h
	toolbarguard.h
	newtabmenumanager.h
	wizardtypechoicepage.h
	plugintreebuilder.h
	coreinstanceobject.h
	settingstab.h
	separatetabbar.h
	separatetabwidget.h
	mwproxy.h
	winwarndialog.h
	coreplugin2manager.h
	dockmanager.h
	acceptlangwidget.h
	entitymanager.h
	interfaces/structures.h
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

IF (USE_QT5)
	QT5_WRAP_UI (UIS_H ${FORMS})
	QT5_ADD_RESOURCES (RCC ${RESOURCES})
ELSE ()
	QT4_WRAP_UI (UIS_H ${FORMS})
	QT4_ADD_RESOURCES (RCC ${RESOURCES})
ENDIF ()

ADD_EXECUTABLE (leechcraft WIN32
	../config.h
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCC}
	)

TARGET_LINK_LIBRARIES (leechcraft
		${QT_QTMAIN_LIBRARY}
		${QT_LIBRARIES}
		${Boost_PROGRAM_OPTIONS_LIBRARY}
		${LEECHCRAFT_LIBRARIES}
		)
INSTALL (TARGETS leechcraft DESTINATION ${LC_BINDIR})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES coresettings.xml DESTINATION ${LC_SETTINGS_DEST})

IF (USE_QT5)
	QT5_USE_MODULES (leechcraft Gui Network Widgets Xml Sql Script Concurrent DBus Svg)
ENDIF ()
