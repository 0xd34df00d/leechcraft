cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_vader)
include (InitLCPlugin NO_POLICY_SCOPE)

option (ENABLE_AZOTH_VADER_VERBOSE_LOG "Debug protocol messages & similar stuff with increased verbosity" OFF)

if (ENABLE_AZOTH_VADER_VERBOSE_LOG)
	add_definitions (-DPROTOCOL_LOGGING)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		vader.cpp
		mrimprotocol.cpp
		mrimaccount.cpp
		mrimaccountconfigwidget.cpp
		mrimbuddy.cpp
		mrimmessage.cpp
		vaderutil.cpp
		groupmanager.cpp
		smsdialog.cpp
		selfavatarfetcher.cpp
		xmlsettingsmanager.cpp
		vcarddialog.cpp
		avatarstimestampstorage.cpp
		proto/headers.cpp
		proto/conversions.cpp
		proto/connection.cpp
		proto/packet.cpp
		proto/packetfactory.cpp
		proto/packetextractor.cpp
		proto/exceptions.cpp
		proto/halfpacket.cpp
		proto/balancer.cpp
		proto/typingmanager.cpp
	RESOURCES azothvaderresources.qrc
	SETTINGS azothvadersettings.xml
	QT_COMPONENTS Network Sql Widgets
	)
