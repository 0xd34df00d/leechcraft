cmake_minimum_required (VERSION 3.10)
project (azoth_acetamide)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
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
		channelsubjecttracker.cpp
		clientconnection.cpp
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
		nickservidentifymanager.cpp
		nickservidentifywidget.cpp
		rplisupportparser.cpp
		parsers.cpp
		servercommandmessage.cpp
		serverinfowidget.cpp
		serverparticipantentry.cpp
		serverresponsemanager.cpp
		usercommandmanager.cpp
		vcarddialog.cpp
	SETTINGS azothacetamidesettings.xml
	QT_COMPONENTS Network Widgets Xml
	LINK_LIBRARIES leechcraft-azoth-util
	INSTALL_DESKTOP
	INSTALL_SHARE
	HAS_TESTS
	)

AddAzothAcetamideTest (urldecoder SRCS tests/urldecodertest.cpp)
