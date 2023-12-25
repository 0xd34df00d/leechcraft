project (poshuku_speeddial)

LC_DEFINE_PLUGIN (
	SRCS
		constants.cpp
		customsitesmanager.cpp
		imagecache.cpp
		pagerenderer.cpp
		requesthandler.cpp
		speeddial.cpp
	SETTINGS poshukuspeeddialsettings.xml
	QT_COMPONENTS Concurrent Widgets
	)
