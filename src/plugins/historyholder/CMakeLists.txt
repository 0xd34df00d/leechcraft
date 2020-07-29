cmake_minimum_required (VERSION 3.10)
project (leechcraft_historyholder)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		historyholder.cpp
		findproxy.cpp
		historydb.cpp
		historyentry.cpp
	RESOURCES historyholderresources.qrc
	QT_COMPONENTS Concurrent Gui Sql
	INSTALL_SHARE
	)
