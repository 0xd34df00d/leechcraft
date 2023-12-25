project (poshuku_onlinebookmarks)

LC_DEFINE_PLUGIN (
	SRCS
		onlinebookmarks.cpp
		core.cpp
		accountssettings.cpp
		pluginmanager.cpp
	SETTINGS poshukuonlinebookmarkssettings.xml
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)

SUBPLUGIN (READITLATER "Build \"Read It Later\" support for OnlineBookmarks")
SUBPLUGIN (DELICIOUS "Build \"Delicious\" support for OnlineBookmarks")
