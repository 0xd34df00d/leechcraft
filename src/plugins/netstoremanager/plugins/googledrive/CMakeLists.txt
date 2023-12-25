cmake_minimum_required (VERSION 3.10)
project (netstoremanager_googledrive)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		account.cpp
		authmanager.cpp
		core.cpp
		drivemanager.cpp
		googledrive.cpp
		uploadmanager.cpp
	SETTINGS nsmgoogledrivesettings.xml
	QT_COMPONENTS Network Widgets
	INSTALL_SHARE
	)
