CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_azoth_xoox)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})

SET (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

FIND_PACKAGE (QXmpp REQUIRED)
FIND_PACKAGE (Speex REQUIRED)

INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${QXMPP_INCLUDE_DIR}
	${QCA2_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
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
	legacyformbuilder.cpp
	inbandaccountregfirstpage.cpp
	inbandaccountregsecondpage.cpp
	inbandaccountregthirdpage.cpp
	legacyentitytimeext.cpp
	pubsubmanager.cpp
	pepeventbase.cpp
	useractivity.cpp
	usermood.cpp
	usertune.cpp
	privacylistsmanager.cpp
	imagemediawidget.cpp
	privacylistsconfigdialog.cpp
	privacylistsitemdialog.cpp
	adhoccommandmanager.cpp
	executecommanddialog.cpp
	userlocation.cpp
	selfcontact.cpp
	adhoccommandserver.cpp
	adhoccommand.cpp
	lastactivitymanager.cpp
	riexmanager.cpp
	jabbersearchsession.cpp
	jabbersearchmanager.cpp
	useravatarmanager.cpp
	useravatardata.cpp
	useravatarmetadata.cpp
	sdmanager.cpp
	msgarchivingmanager.cpp
	avatarsstorage.cpp
	xep0232handler.cpp
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
	legacyformbuilder.h
	inbandaccountregfirstpage.h
	inbandaccountregsecondpage.h
	inbandaccountregthirdpage.h
	legacyentitytimeext.h
	pubsubmanager.h
	pepeventbase.h
	useractivity.h
	usermood.h
	usertune.h
	privacylistsmanager.h
	imagemediawidget.h
	privacylistsconfigdialog.h
	privacylistsitemdialog.h
	adhoccommandmanager.h
	executecommanddialog.h
	userlocation.h
	selfcontact.h
	adhoccommandserver.h
	adhoccommand.h
	lastactivitymanager.h
	riexmanager.h
	jabbersearchsession.h
	jabbersearchmanager.h
	useravatarmanager.h
	useravatardata.h
	useravatarmetadata.h
	sdmanager.h
	msgarchivingmanager.h
	avatarsstorage.h
	xep0232handler.h
	)
SET (XOOX_FORMS
	glooxaccountconfigurationdialog.ui
	glooxaccountconfigurationwidget.ui
	joingroupchatwidget.ui
	vcarddialog.ui
	bookmarkeditwidget.ui
	roomconfigwidget.ui
	affiliationselectordialog.ui
	inbandaccountregfirstpage.ui
	privacylistsconfigdialog.ui
	privacylistsitemdialog.ui
	executecommanddialog.ui
	commandslistpage.ui
	commandresultpage.ui
	)
SET (XOOX_RESOURCES azothxooxresources.qrc)

IF (ENABLE_CRYPT)
	ADD_DEFINITIONS (-DENABLE_CRYPT)

	SET (XOOX_SRCS ${XOOX_SRCS} pgpmanager.cpp)
	SET (XOOX_HEADERS ${XOOX_HEADERS} pgpmanager.h)
ENDIF (ENABLE_CRYPT)

IF (ENABLE_MEDIACALLS)
	ADD_DEFINITIONS (-DENABLE_MEDIACALLS)

	SET (XOOX_SRCS ${XOOX_SRCS} mediacall.cpp)
	SET (XOOX_HEADERS ${XOOX_HEADERS} mediacall.h)
ENDIF (ENABLE_MEDIACALLS)

CreateTrVars ("azoth_xoox" "de;en;es;fr;it;ru_RU;uk_UA" XOOX_TRANSLATIONS XOOX_COMPILED_TRANSLATIONS)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (XOOX_MOC_SRCS ${XOOX_HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (XOOX_UIS_H ${XOOX_FORMS})
QT4_ADD_RESOURCES (XOOX_RCCS ${XOOX_RESOURCES})

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
	${XOOX_RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth_xoox
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QXMPP_LIBRARIES}
	${SPEEX_LIBRARIES}
	${QCA2_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_azoth_xoox DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${XOOX_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
