cmake_minimum_required (VERSION 3.10)
project (leechcraft_imgaste)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		imgaste.cpp
		hostingservice.cpp
		poster.cpp
		requestbuilder.cpp
	QT_COMPONENTS Network Widgets
	)
