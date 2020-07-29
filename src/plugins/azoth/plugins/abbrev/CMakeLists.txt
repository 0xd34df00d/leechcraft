cmake_minimum_required (VERSION 3.10)
project (leechcraft_azoth_abbrev)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		abbrev.cpp
		abbrevsmanager.cpp
		abbreviation.cpp
		shortcutsmanager.cpp
	QT_COMPONENTS Core Widgets
	)
