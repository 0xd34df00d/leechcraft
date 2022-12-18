cmake_minimum_required (VERSION 3.10)
project (pogooglue)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS pogooglue.cpp
	RESOURCES pogooglueresources.qrc
	QT_COMPONENTS Gui
	)
