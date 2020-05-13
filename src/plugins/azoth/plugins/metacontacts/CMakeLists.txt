cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_metacontacts)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (azoth_metacontacts
	SRCS
		metacontacts.cpp
		metaprotocol.cpp
		metaaccount.cpp
		metaentry.cpp
		metamessage.cpp
		core.cpp
		addtometacontactsdialog.cpp
		managecontactsdialog.cpp
	FORMS
		addtometacontactsdialog.ui
		managecontactsdialog.ui
	QT_COMPONENTS Widgets
	TS_LANGUAGES en es ro ru_RU uk_UA
	INSTALL_SHARE
	)
