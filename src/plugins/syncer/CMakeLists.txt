cmake_minimum_required (VERSION 3.10)
project (syncer)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (LibLaretz REQUIRED)

LC_DEFINE_PLUGIN (
	SRCS
		syncer.cpp
		syncablemanager.cpp
		singlesyncable.cpp
	SETTINGS syncersettings.xml
	QT_COMPONENTS Gui Network
	LINK_LIBRARIES ${LARETZ_LIBRARIES}
	)
