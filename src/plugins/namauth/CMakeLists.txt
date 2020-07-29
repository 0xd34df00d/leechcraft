cmake_minimum_required (VERSION 3.10)
project (leechcraft_namauth)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		authenticationdialog.cpp
		namauth.cpp
		namhandler.cpp
		sqlstoragebackend.cpp
	QT_COMPONENTS Network Sql Widgets
	)
