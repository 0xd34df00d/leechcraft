cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_xoox)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package(PkgConfig)
pkg_check_modules(QXMPP REQUIRED IMPORTED_TARGET qxmpp)

include_directories (${QCA2_INCLUDE_DIR})
set (DEFINITIONS "")
set (EXTRA_SRCS "")

if (ENABLE_CRYPT)
	list (APPEND DEFINITIONS "-DENABLE_CRYPT")
	list (APPEND EXTRA_SRCS xeps/pgpmanager.cpp)
endif ()

if (ENABLE_MEDIACALLS)
	list (APPEND DEFINITIONS "-DENABLE_MEDIACALLS")
	list (APPEND EXTRA_SRCS mediacall.cpp)
endif ()

add_definitions (${DEFINITIONS})
set (CMAKE_AUTOMOC_MOC_OPTIONS ${CMAKE_AUTOMOC_MOC_OPTIONS} ${DEFINITIONS})

set (EXTRA_QT)
if (ENABLE_MEDIACALLS)
	set (EXTRA_QT Multimedia)
endif ()

LC_DEFINE_PLUGIN (azoth_xoox
	SRCS
		xoox.cpp
		glooxprotocol.cpp
		glooxaccountconfigurationdialog.cpp
		glooxaccountconfigurationwidget.cpp
		glooxaccount.cpp
		clientconnection.cpp
		clientconnectionextensionsmanager.cpp
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
		capsstorageondisk.cpp
		sdsession.cpp
		sdmodel.cpp
		affiliationselectordialog.cpp
		annotationsmanager.cpp
		fetchqueue.cpp
		legacyformbuilder.cpp
		inbandaccountregfirstpage.cpp
		inbandaccountregsecondpage.cpp
		inbandaccountregthirdpage.cpp
		xeps/legacyentitytimeext.cpp
		xeps/pubsubmanager.cpp
		pepeventbase.cpp
		useractivity.cpp
		usermood.cpp
		usertune.cpp
		xeps/privacylistsmanager.cpp
		imagemediawidget.cpp
		privacylistsconfigdialog.cpp
		privacylistsitemdialog.cpp
		xeps/adhoccommandmanager.cpp
		executecommanddialog.cpp
		userlocation.cpp
		selfcontact.cpp
		xeps/adhoccommandserver.cpp
		adhoccommand.cpp
		xeps/lastactivitymanager.cpp
		xeps/riexmanager.cpp
		jabbersearchsession.cpp
		xeps/jabbersearchmanager.cpp
		useravatarmanager.cpp
		useravatardata.cpp
		useravatarmetadata.cpp
		sdmanager.cpp
		xeps/msgarchivingmanager.cpp
		xep0232handler.cpp
		pepmicroblog.cpp
		vcardlisteditdialog.cpp
		regformhandlerwidget.cpp
		gwoptionsdialog.cpp
		gwitemsremovaldialog.cpp
		clientconnectionerrormgr.cpp
		accstatusrestorer.cpp
		accountsettingsholder.cpp
		crypthandler.cpp
		serverinfostorage.cpp
		inforequestpolicymanager.cpp
		xeps/xmppannotationsiq.cpp
		xeps/xmppannotationsmanager.cpp
		xeps/xmppbobiq.cpp
		xeps/xmppbobmanager.cpp
		xeps/xmppcaptchaiq.cpp
		xeps/xmppcaptchamanager.cpp
		xmlsettingsmanager.cpp
		captchamanager.cpp
		xeps/xep0313manager.cpp
		xeps/xep0313prefiq.cpp
		xep0313prefsdialog.cpp
		xep0313modelmanager.cpp
		xeps/xep0313reqiq.cpp
		xep0334utils.cpp
		xeps/carbonsmanager.cpp
		xeps/pingmanager.cpp
		pingreplyobject.cpp
		pendingversionquery.cpp
		pendinglastactivityrequest.cpp
		discomanagerwrapper.cpp
		addtoblockedrunner.cpp
		rostersaver.cpp
		vcardstorage.cpp
		vcardstorageondisk.cpp
		vcardstorageondiskwriter.cpp
		sslerrorshandler.cpp
		offlinedatasource.cpp
		callshandler.cpp
		bookmarksintegrator.cpp
		clientloggermanager.cpp
		riexintegrator.cpp
		inbandaccountactions.cpp
		xmppelementdescription.cpp
		deliveryreceiptsintegrator.cpp
		${EXTRA_SRCS}
	FORMS
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
		vcardlisteditdialog.ui
		gwoptionsdialog.ui
		gwitemsremovaldialog.ui
		xep0313prefsdialog.ui
	RESOURCES azothxooxresources.qrc
	SETTINGS azothxooxsettings.xml
	QT_COMPONENTS Concurrent Network Sql Widgets Xml ${EXTRA_QT}
	LINK_LIBRARIES ${QCA2_LIBRARIES} PkgConfig::QXMPP
	INSTALL_DESKTOP
	)
