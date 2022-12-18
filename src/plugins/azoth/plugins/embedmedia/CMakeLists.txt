cmake_minimum_required (VERSION 3.10)
project (azoth_embedmedia)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS embedmedia.cpp
	QT_COMPONENTS WebEngineWidgets
	INSTALL_SHARE
	)
