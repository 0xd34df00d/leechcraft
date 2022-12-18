project (poshuku_speeddial)

LC_DEFINE_PLUGIN (
	SRCS
		speeddial.cpp
		viewhandler.cpp
		imagecache.cpp
		customsitesmanager.cpp
		xmlsettingsmanager.cpp
	SETTINGS poshukuspeeddialsettings.xml
	QT_COMPONENTS Concurrent Widgets
	)
