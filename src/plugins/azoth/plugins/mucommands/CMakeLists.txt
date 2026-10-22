cmake_minimum_required (VERSION 3.10)
project (azoth_mucommands)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		mucommands.cpp
		commands.cpp
		presencecommand.cpp
		openurlcommand.cpp
		descparser.cpp
	QT_COMPONENTS Widgets
	LINK_LIBRARIES leechcraft-azoth-util
	HAS_TESTS
	)

AddAzothMucommandsTest (presence SRCS tests/presencecommandtest.cpp)
AddAzothMucommandsTest (openurl SRCS tests/openurlcommandtest.cpp)
