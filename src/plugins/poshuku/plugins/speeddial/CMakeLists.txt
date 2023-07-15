project (poshuku_speeddial)

LC_DEFINE_PLUGIN (
	SRCS
		speeddial.cpp
		imagecache.cpp
		customsitesmanager.cpp
		xmlsettingsmanager.cpp
		requesthandler.cpp
	SETTINGS poshukuspeeddialsettings.xml
	QT_COMPONENTS Concurrent Widgets
	)
