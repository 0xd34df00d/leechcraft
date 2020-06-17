cmake_minimum_required (VERSION 3.3)
project (leechcraft_nacheku)
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
