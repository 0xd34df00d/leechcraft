cmake_minimum_required (VERSION 3.10)
project (azoth_nativeemoticons)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		nativeemoticons.cpp
		nativeemoticonssource.cpp
		kopeteemoticonssource.cpp
		psiplusemoticonssource.cpp
		baseemoticonssource.cpp
	QT_COMPONENTS Xml Widgets
	INSTALL_SHARE
	)
