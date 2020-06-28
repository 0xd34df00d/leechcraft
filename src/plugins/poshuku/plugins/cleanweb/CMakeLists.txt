project (leechcraft_poshuku_cleanweb)

LC_DEFINE_PLUGIN (
	SRCS
		cleanweb.cpp
		core.cpp
		xmlsettingsmanager.cpp
		subscriptionsmanagerwidget.cpp
		userfilters.cpp
		userfiltersmodel.cpp
		filter.cpp
		ruleoptiondialog.cpp
		wizardgenerator.cpp
		startupfirstpage.cpp
		subscriptionadddialog.cpp
		lineparser.cpp
		subscriptionsmodel.cpp
	SETTINGS poshukucleanwebsettings.xml
	QT_COMPONENTS Concurrent Widgets Xml
	INSTALL_SHARE
	)
