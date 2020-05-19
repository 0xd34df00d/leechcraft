cmake_minimum_required (VERSION 3.3)
project (leechcraft_auscrie)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (auscrie
	SRCS
		auscrie.cpp
		shooterdialog.cpp
		util.cpp
	FORMS shooterdialog.ui
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)
