cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_autopaste)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (azoth_autopaste
	SRCS
		actionsstorage.cpp
		autopaste.cpp
		highlight2str.cpp
		bpasteservice.cpp
		codepadservice.cpp
		hastebinservice.cpp
		pasteorgruservice.cpp
		spacepasteservicebase.cpp
		pastedialog.cpp
		pasteservicebase.cpp
		pasteservicefactory.cpp
		xmlsettingsmanager.cpp
	FORMS
		pastedialog.ui
	SETTINGS azothautopastesettings.xml
	QT_COMPONENTS Network Widgets
	INSTALL_SHARE
	)
