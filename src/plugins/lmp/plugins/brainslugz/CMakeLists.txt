cmake_minimum_required (VERSION 3.10)
project (lmp_brainslugz)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		brainslugz.cpp
		checktab.cpp
		checkmodel.cpp
		checker.cpp
		progressmodelmanager.cpp
	QT_COMPONENTS Concurrent QuickWidgets
	INSTALL_SHARE
	)
