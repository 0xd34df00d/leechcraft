project (poshuku_fatape)

LC_DEFINE_PLUGIN (
	SRCS
		createscriptdialog.cpp
		fatape.cpp
		greasemonkey.cpp
		userscript.cpp
		userscriptcreator.cpp
		userscriptinstallerdialog.cpp
		userscriptsmanagerwidget.cpp
	SETTINGS poshukufatapesettings.xml
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)
