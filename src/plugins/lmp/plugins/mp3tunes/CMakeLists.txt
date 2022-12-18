cmake_minimum_required (VERSION 3.10)
project (lmp_mp3tunes)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		mp3tunes.cpp
		accountsmanager.cpp
		authmanager.cpp
		playlistmanager.cpp
		uploader.cpp
		xmlsettingsmanager.cpp
	SETTINGS lmpmp3tunessettings.xml
	QT_COMPONENTS Gui Network Xml
	)
