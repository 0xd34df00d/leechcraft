cmake_minimum_required (VERSION 3.10)
project (blasq_rappor)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		rappor.cpp
		vkservice.cpp
		vkaccount.cpp
		albumsettingsdialog.cpp
		uploadmanager.cpp
	RESOURCES blasqrapporresources.qrc
	QT_COMPONENTS Network Widgets Xml
	)
