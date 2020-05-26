cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_embedmedia)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS embedmedia.cpp
	QT_COMPONENTS WebKitWidgets
	INSTALL_SHARE
	)
