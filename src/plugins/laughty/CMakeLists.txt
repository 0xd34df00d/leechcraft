cmake_minimum_required (VERSION 3.10)
project (laughty)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		laughty.cpp
		serverobject.cpp
		serveradaptor.cpp
	QT_COMPONENTS DBus Gui
	)
