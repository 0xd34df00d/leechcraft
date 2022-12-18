cmake_minimum_required (VERSION 3.10)
project (nacheku)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		nacheku.cpp
		xmlsettingsmanager.cpp
		clipboardwatcher.cpp
		directorywatcher.cpp
	SETTINGS nachekusettings.xml
	QT_COMPONENTS Widgets
	)
