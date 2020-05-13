cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_isterique)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (azoth_isterique
	SRCS
		isterique.cpp
		xmlsettingsmanager.cpp
	SETTINGS azothisteriquesettings.xml
	QT_COMPONENTS Core Widgets
	TS_LANGUAGES en ru_RU uk_UA
	INSTALL_SHARE
	)
