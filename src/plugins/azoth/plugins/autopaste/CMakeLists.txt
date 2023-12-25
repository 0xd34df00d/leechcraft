cmake_minimum_required (VERSION 3.10)
project (azoth_autopaste)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
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
	SETTINGS azothautopastesettings.xml
	QT_COMPONENTS Network Widgets
	INSTALL_SHARE
	)
