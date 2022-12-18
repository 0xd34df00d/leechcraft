cmake_minimum_required (VERSION 3.10)
project (azoth_otroid)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (OTR REQUIRED IMPORTED_TARGET libotr>=4.0.0)

LC_DEFINE_PLUGIN (
	SRCS
		otroid.cpp
		otrhandler.cpp
		authenticator.cpp
		initiateauthdialog.cpp
		fpmanager.cpp
		privkeymanager.cpp
		xmlsettingsmanager.cpp
		util.cpp
	SETTINGS azothotroidsettings.xml
	QT_COMPONENTS Concurrent Widgets
	LINK_LIBRARIES PkgConfig::OTR
	INSTALL_SHARE
	)
