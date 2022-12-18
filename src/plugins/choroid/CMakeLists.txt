cmake_minimum_required (VERSION 3.10)
project (choroid)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		choroid.cpp
		choroidtab.cpp
	QT_COMPONENTS QuickWidgets Widgets
	INSTALL_SHARE
	)
