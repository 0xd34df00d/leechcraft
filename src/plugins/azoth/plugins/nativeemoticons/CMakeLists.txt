cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_nativeemoticons)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (azoth_nativeemoticons
	SRCS
		nativeemoticons.cpp
		nativeemoticonssource.cpp
		kopeteemoticonssource.cpp
		psiplusemoticonssource.cpp
		baseemoticonssource.cpp
	QT_COMPONENTS Xml Widgets
	INSTALL_SHARE
	)
