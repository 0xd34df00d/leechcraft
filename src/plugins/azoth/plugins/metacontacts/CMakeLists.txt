cmake_minimum_required (VERSION 3.10)
project (azoth_metacontacts)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		metacontacts.cpp
		metaprotocol.cpp
		metaaccount.cpp
		metaentry.cpp
		metamessage.cpp
		core.cpp
		addtometacontactsdialog.cpp
		managecontactsdialog.cpp
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)
