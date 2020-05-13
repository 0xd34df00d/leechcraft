cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_birthdaynotifier)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (azoth_birthdaynotifier
	SRCS
		birthdaynotifier.cpp
		xmlsettingsmanager.cpp
	SETTINGS azothbirthdaynotifiersettings.xml
	QT_COMPONENTS Core Widgets
	TS_LANGUAGES en ro ru_RU
	INSTALL_SHARE
	)
