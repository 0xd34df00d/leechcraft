cmake_minimum_required (VERSION 3.10)
project (blasq_spegnersi)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		spegnersi.cpp
		flickrservice.cpp
		flickraccount.cpp
	RESOURCES blasqspegnersiresources.qrc
	QT_COMPONENTS Gui Network NetworkAuth Xml
	)
