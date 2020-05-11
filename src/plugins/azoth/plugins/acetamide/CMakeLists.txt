cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_acetamide)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${AZOTH_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

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
	FORMS
		bookmarkeditwidget.ui
		channelconfigwidget.ui
		channelslistdialog.ui
		ircaccountconfigurationdialog.ui
		ircaccountconfigurationwidget.ui
		ircjoingroupchat.ui
		invitechannelsdialog.ui
		newnickservidentifydialog.ui
		nickservidentifywidget.ui
		serverinfowidget.ui
		vcarddialog.ui
	SETTINGS azothacetamidesettings.xml
	RESOURCES azothacetamideresources.qrc
	QT_COMPONENTS Network Widgets Xml
	TS_LANGUAGES de en es fr it ro ru_RU uk_UA
	INSTALL_DESKTOP
	INSTALL_SHARE
	)
