SET (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

FIND_PACKAGE (QXmpp REQUIRED)

INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${QXMPP_INCLUDE_DIR})
SET (XOOX_SRCS
	xoox.cpp
	glooxprotocol.cpp
	glooxaccountconfigurationdialog.cpp
	glooxaccountconfigurationwidget.cpp
	glooxaccount.cpp
	clientconnection.cpp
	glooxclentry.cpp
	glooxmessage.cpp
	joingroupchatwidget.cpp
	roomhandler.cpp
	roomclentry.cpp
	roompublicmessage.cpp
	roomparticipantentry.cpp
	entrybase.cpp
	util.cpp
	core.cpp
	unauthclentry.cpp
	vcarddialog.cpp
	transfermanager.cpp
	transferjob.cpp
	bookmarkeditwidget.cpp
	formbuilder.cpp
	roomconfigwidget.cpp
	capsmanager.cpp
	capsdatabase.cpp
	sdsession.cpp
	sdmodel.cpp
	affiliationselectordialog.cpp
	annotationsmanager.cpp
	fetchqueue.cpp
	)
SET (XOOX_HEADERS
	xoox.h
	glooxprotocol.h
	glooxaccountconfigurationdialog.h
	glooxaccountconfigurationwidget.h
	glooxaccount.h
	clientconnection.h
	glooxclentry.h
	glooxmessage.h
	joingroupchatwidget.h
	roomhandler.h
	roomclentry.h
	roompublicmessage.h
	roomparticipantentry.h
	entrybase.h
	util.h
	core.h
	unauthclentry.h
	vcarddialog.h
	transfermanager.h
	transferjob.h
	bookmarkeditwidget.h
	formbuilder.h
	roomconfigwidget.h
	capsmanager.h
	capsdatabase.h
	sdsession.h
	sdmodel.h
	affiliationselectordialog.h
	annotationsmanager.h
	fetchqueue.h
	)
SET (XOOX_FORMS
	glooxaccountconfigurationdialog.ui
	glooxaccountconfigurationwidget.ui
	joingroupchatwidget.ui
	vcarddialog.ui
	bookmarkeditwidget.ui
	roomconfigwidget.ui
	affiliationselectordialog.ui
	)
CreateTrVars ("azoth_xoox" "de;en;fr;it;ru_RU;uk_UA" XOOX_TRANSLATIONS XOOX_COMPILED_TRANSLATIONS)
QT4_WRAP_CPP (XOOX_MOC_SRCS ${XOOX_HEADERS})
QT4_WRAP_UI (XOOX_UIS_H ${XOOX_FORMS})

ADD_CUSTOM_COMMAND (OUTPUT
	${XOOX_COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${XOOX_TRANSLATIONS}
	DEPENDS ${XOOX_TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_azoth_xoox SHARED
	${XOOX_COMPILED_TRANSLATIONS}
	${XOOX_SRCS}
	${XOOX_MOC_SRCS}
	${XOOX_UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth_xoox
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	${QXMPP_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_azoth_xoox DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${XOOX_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
