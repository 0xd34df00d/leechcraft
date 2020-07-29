cmake_minimum_required (VERSION 3.10)
project (leechcraft_launchy)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		launchy.cpp
		fsdisplayer.cpp
		itemssortfilterproxymodel.cpp
		favoritesmanager.cpp
		quarkmanager.cpp
		itemimageprovider.cpp
		syspathitemprovider.cpp
		recentmanager.cpp
	QT_COMPONENTS QuickWidgets
	INSTALL_SHARE
	)
