project (poshuku_cleanweb)

LC_DEFINE_PLUGIN (
	SRCS
		cleanweb.cpp
		core.cpp
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
		pslhandler.cpp
		pslfetcher.cpp
	SETTINGS poshukucleanwebsettings.xml
	QT_COMPONENTS Concurrent Widgets Xml
	INSTALL_SHARE
	HAS_TESTS
	)

AddPoshukuCleanwebTest (pslhandler SRCS tests/pslhandlertest.cpp)
