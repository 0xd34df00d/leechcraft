cmake_minimum_required (VERSION 3.10)
project (netstoremanager_dbox)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		account.cpp
		authmanager.cpp
		chunkiodevice.cpp
		core.cpp
		drivemanager.cpp
		dropbox.cpp
		uploadmanager.cpp
		xmlsettingsmanager.cpp
	SETTINGS nsmdropboxsettings.xml
	QT_COMPONENTS Network Widgets
	INSTALL_SHARE
	)
