cmake_minimum_required (VERSION 3.3)
project (leechcraft_blasq)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		blasq.cpp
		xmlsettingsmanager.cpp
		accountswidget.cpp
		newaccountwizardfirstpage.cpp
		servicesmanager.cpp
		accountsmanager.cpp
		photostab.cpp
		defaultimagechooser.cpp
		enumsproxy.cpp
		uploadphotosdialog.cpp
		selectalbumdialog.cpp
		photosproxymodel.cpp
		datafilteruploader.cpp
	SETTINGS blasqsettings.xml
	QT_COMPONENTS Network QuickWidgets Widgets
	INSTALL_SHARE
	)

SUBPLUGIN (RAPPOR "Build VKontakte client module for Blasq")
SUBPLUGIN (SPEGNERSI "Build Flickr client module for Blasq")
SUBPLUGIN (VANGOG "Build Picasa client module for Blasq")
SUBPLUGIN (DEATHNOTE "Build LiveJournal FotoBilder client module for Blasq")
