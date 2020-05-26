cmake_minimum_required (VERSION 3.3)
project (leechcraft_blasq_spegnersi)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (QtKOAuth REQUIRED)

LC_DEFINE_PLUGIN (
	SRCS
		spegnersi.cpp
		flickrservice.cpp
		flickraccount.cpp
	RESOURCES blasqspegnersiresources.qrc
	QT_COMPONENTS Gui Network Xml
	LINK_LIBRARIES ${QTKOAUTH_LIBRARIES}
	)
