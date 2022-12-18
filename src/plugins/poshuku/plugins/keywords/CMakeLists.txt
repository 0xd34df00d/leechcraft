project (poshuku_keywords)

LC_DEFINE_PLUGIN (
	SRCS
		keywords.cpp
		keywordsmanagerwidget.cpp
		editkeyworddialog.cpp
		xmlsettingsmanager.cpp
	SETTINGS poshukukeywordssettings.xml
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)
