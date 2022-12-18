cmake_minimum_required (VERSION 3.10)
project (blasq_vangog)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		albumsettingsdialog.cpp
		authmanager.cpp
		picasaaccount.cpp
		picasamanager.cpp
		picasaservice.cpp
		registerpage.cpp
		uploadmanager.cpp
		vangog.cpp
	RESOURCES blasqvangogresources.qrc
	QT_COMPONENTS Network Widgets Xml
	)
