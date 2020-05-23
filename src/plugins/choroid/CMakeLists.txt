cmake_minimum_required (VERSION 3.3)
project (leechcraft_choroid)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (choroid
	SRCS
		choroid.cpp
		choroidtab.cpp
	FORMS choroidtab.ui
	QT_COMPONENTS QuickWidgets Widgets
	INSTALL_SHARE
	)
