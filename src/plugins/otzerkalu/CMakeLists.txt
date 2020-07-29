cmake_minimum_required (VERSION 3.10)
project (leechcraft_otzerkalu)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		otzerkalu.cpp
		otzerkaludialog.cpp
		otzerkaludownloader.cpp
	QT_COMPONENTS WebKitWidgets
	)
