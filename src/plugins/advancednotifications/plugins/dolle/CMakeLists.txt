cmake_minimum_required (VERSION 3.10)
project (leechcraft_advancednotifications_dolle)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		dolle.cpp
		notificationhandler.cpp
		dockutil.mm
		lcbadgeview.mm
	QT_COMPONENTS Gui
	LINK_LIBRARIES "-framework AppKit"
	INSTALL_SHARE
	)
