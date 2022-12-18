cmake_minimum_required (VERSION 3.10)
project (blasq_deathnote)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		albumsettingsdialog.cpp
		deathnote.cpp
		fotobilderaccount.cpp
		fotobilderservice.cpp
		registerpage.cpp
		selectgroupsdialog.cpp
		util.cpp
	RESOURCES blasqdeathnoteresources.qrc
	QT_COMPONENTS Network Widgets Xml XmlPatterns
	)
