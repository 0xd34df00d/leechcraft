cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_acetamide)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (azoth_acetamide
	SRCS
		acetamide.cpp
		bookmarkeditwidget.cpp
		channelclentry.cpp
		channelconfigwidget.cpp
		channelhandler.cpp
		channelparticipantentry.cpp
		channelpublicmessage.cpp
		channelslistdialog.cpp
		channelslistfilterproxymodel.cpp
		channelsmanager.cpp
		clientconnection.cpp
		core.cpp
		entrybase.cpp
		ircaccount.cpp
		ircaccountconfigurationdialog.cpp
		ircaccountconfigurationwidget.cpp
		ircerrorhandler.cpp
		ircjoingroupchat.cpp
		ircmessage.cpp
		ircparser.cpp
		ircparticipantentry.cpp
		ircprotocol.cpp
		ircserversocket.cpp
		ircserverclentry.cpp
		ircserverhandler.cpp
		invitechannelsdialog.cpp
		localtypes.cpp
		newnickservidentifydialog.cpp
		nickservidentifywidget.cpp
		rplisupportparser.cpp
		servercommandmessage.cpp
		serverinfowidget.cpp
		serverparticipantentry.cpp
		serverresponsemanager.cpp
		sortfilterproxymodel.cpp
		usercommandmanager.cpp
		xmlsettingsmanager.cpp
		vcarddialog.cpp
	SETTINGS azothacetamidesettings.xml
	RESOURCES azothacetamideresources.qrc
	QT_COMPONENTS Network Widgets Xml
	INSTALL_DESKTOP
	INSTALL_SHARE
	)
