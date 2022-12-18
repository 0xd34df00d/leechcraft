project (poshuku_webkitview)

LC_DEFINE_PLUGIN (
	SRCS
		customwebpage.cpp
		customwebview.cpp
		externalproxy.cpp
		interceptadaptor.cpp
		jsproxy.cpp
		linkhistory.cpp
		pluginmanager.cpp
		settingsglobalhandler.cpp
		settingsinstancehandler.cpp
		sslstatedialog.cpp
		webkitview.cpp
		webpagesslwatcher.cpp
		webpluginfactory.cpp
		webviewrendersettingshandler.cpp
		webviewsmoothscroller.cpp
		webviewsslwatcherhandler.cpp
		xmlsettingsmanager.cpp
	SETTINGS poshukuwebkitviewsettings.xml
	QT_COMPONENTS PrintSupport WebKitWidgets
	)
