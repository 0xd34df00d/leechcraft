cmake_minimum_required (VERSION 3.10)
project (azoth_otroid)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (OTR REQUIRED IMPORTED_TARGET libotr>=4.0.0)
pkg_check_modules (GCRYPT REQUIRED IMPORTED_TARGET libgcrypt)

LC_DEFINE_PLUGIN (
	SRCS
		otroid.cpp
		otrhandler.cpp
		authenticator.cpp
		initiateauthdialog.cpp
		fpmanager.cpp
		privkeymanager.cpp
		util.cpp
	SETTINGS azothotroidsettings.xml
	QT_COMPONENTS Concurrent Widgets
	LINK_LIBRARIES leechcraft-azoth-util PkgConfig::OTR PkgConfig::GCRYPT
	INSTALL_SHARE
	)
