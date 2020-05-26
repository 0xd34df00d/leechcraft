cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_velvetbird)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (Purple REQUIRED IMPORTED_TARGET purple)

LC_DEFINE_PLUGIN (
	SRCS
		velvetbird.cpp
		protomanager.cpp
		protocol.cpp
		account.cpp
		accregfirstpage.cpp
		util.cpp
		buddy.cpp
		convimmessage.cpp
		accountconfigdialog.cpp
	QT_COMPONENTS Widgets
	LINK_LIBRARIES PkgConfig::Purple
	INSTALL_SHARE
	)
