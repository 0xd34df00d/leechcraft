cmake_minimum_required (VERSION 3.10)
project (leechcraft_netstoremanager_googledrive)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		account.cpp
		authmanager.cpp
		core.cpp
		drivemanager.cpp
		googledrive.cpp
		uploadmanager.cpp
		xmlsettingsmanager.cpp
	SETTINGS nsmgoogledrivesettings.xml
	QT_COMPONENTS Network Widgets
	INSTALL_SHARE
	)
