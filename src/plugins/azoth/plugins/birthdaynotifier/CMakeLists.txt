cmake_minimum_required (VERSION 3.10)
project (azoth_birthdaynotifier)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		birthdaynotifier.cpp
		xmlsettingsmanager.cpp
	SETTINGS azothbirthdaynotifiersettings.xml
	QT_COMPONENTS Core Widgets
	INSTALL_SHARE
	)
