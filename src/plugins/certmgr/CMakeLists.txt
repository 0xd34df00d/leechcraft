cmake_minimum_required (VERSION 3.10)
project (leechcraft_certmgr)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		certmgr.cpp
		xmlsettingsmanager.cpp
		acceptedrejecteddialog.cpp
		manager.cpp
		certsmodel.cpp
		managerdialog.cpp
		exceptionsmodel.cpp
	SETTINGS certmgrsettings.xml
	QT_COMPONENTS Network Widgets
	)
