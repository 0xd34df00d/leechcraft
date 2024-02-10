cmake_minimum_required (VERSION 3.10)
project (monocle_fxb)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		fxb.cpp
		document.cpp
		fb2converter.cpp
		tocbuilder.cpp
	SETTINGS monoclefxbsettings.xml
	QT_COMPONENTS Xml
	LINK_LIBRARIES leechcraft-monocle-util
	INSTALL_DESKTOP
	)
