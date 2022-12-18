cmake_minimum_required (VERSION 3.10)
project (blasq_spegnersi)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (KQOAuth REQUIRED IMPORTED_TARGET kqoauth)

LC_DEFINE_PLUGIN (
	SRCS
		spegnersi.cpp
		flickrservice.cpp
		flickraccount.cpp
	RESOURCES blasqspegnersiresources.qrc
	QT_COMPONENTS Gui Network Xml
	LINK_LIBRARIES PkgConfig::KQOAuth
	)
