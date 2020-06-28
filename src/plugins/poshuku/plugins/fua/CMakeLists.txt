project (leechcraft_poshuku_fua)

LC_DEFINE_PLUGIN (
	SRCS
		fua.cpp
		settings.cpp
		changer.cpp
		xmlsettingsmanager.cpp
	RESOURCES poshukufuaresources.qrc
	SETTINGS poshukufuasettings.xml
	QT_COMPONENTS Widgets
	)
