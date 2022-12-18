cmake_minimum_required (VERSION 3.10)
project (azoth_xoox)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (QXMPP REQUIRED IMPORTED_TARGET qxmpp>=1.3.0)

add_compile_definitions (
	$<$<BOOL:${ENABLE_CRYPT}>:ENABLE_CRYPT>
	$<$<BOOL:${ENABLE_MEDIACALLS}>:ENABLE_MEDIACALLS>
	QXMPP_DISABLE_DEPRECATED_BEFORE=0x010400
	)

LC_DEFINE_PLUGIN (
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
		sslerrorshandler.cpp
		offlinedatasource.cpp
		callshandler.cpp
		bookmarksintegrator.cpp
		clientloggermanager.cpp
		riexintegrator.cpp
		inbandaccountactions.cpp
		xmppelementdescription.cpp
		deliveryreceiptsintegrator.cpp
		$<$<BOOL:${ENABLE_CRYPT}>:xeps/pgpmanager.cpp>
		$<$<BOOL:${ENABLE_MEDIACALLS}>:mediacall.cpp>
	RESOURCES azothxooxresources.qrc
	SETTINGS azothxooxsettings.xml
	QT_COMPONENTS Concurrent Network Sql Widgets Xml ${EXTRA_MULTIMEDIA_LIBRARY}
	LINK_LIBRARIES PkgConfig::QXMPP $<$<BOOL:${ENABLE_CRYPT}>:PkgConfig::QCA2>
	INSTALL_DESKTOP
	)

