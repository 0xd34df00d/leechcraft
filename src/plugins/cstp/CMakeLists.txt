cmake_minimum_required (VERSION 3.3)
project (leechcraft_cstp)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		cstp.cpp
		core.cpp
		task.cpp
		addtask.cpp
		xmlsettingsmanager.cpp
	SETTINGS cstpsettings.xml
	QT_COMPONENTS Network Widgets
	INSTALL_SHARE
	)
