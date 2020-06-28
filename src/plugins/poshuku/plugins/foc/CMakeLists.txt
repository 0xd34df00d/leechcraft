project (leechcraft_poshuku_foc)

LC_DEFINE_PLUGIN (
	SRCS
		flashplaceholder.cpp
		flashonclickplugin.cpp
		flashonclickwhitelist.cpp
		foc.cpp
		xmlsettingsmanager.cpp
	SETTINGS poshukufocsettings.xml
	QT_COMPONENTS Widgets WebKitWidgets
	)
