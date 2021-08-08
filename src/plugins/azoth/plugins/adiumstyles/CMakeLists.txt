cmake_minimum_required (VERSION 3.10)
project (leechcraft_azoth_adiumstyles)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		adiumstyles.cpp
		adiumstylesource.cpp
		packproxymodel.cpp
	RESOURCES adiumstyles.qrc
	QT_COMPONENTS WebEngineWidgets Xml
	INSTALL_SHARE
	)
