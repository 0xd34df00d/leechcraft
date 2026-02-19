cmake_minimum_required (VERSION 3.10)
project (summary)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		summary.cpp
		summarywidget.cpp
		jobspresentationmodel.cpp
	INSTALL_SHARE
	QT_COMPONENTS Widgets
	)
