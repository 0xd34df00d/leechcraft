cmake_minimum_required (VERSION 3.10)
project (leechcraft_summary)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		summary.cpp
		core.cpp
		summarywidget.cpp
		summarytagsfilter.cpp
		modeldelegate.cpp
	INSTALL_SHARE
	QT_COMPONENTS Widgets
	)
