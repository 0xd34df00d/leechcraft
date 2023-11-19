cmake_minimum_required (VERSION 3.10)
project (imgaste)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	PLUGIN_VISIBLE_NAME Imgaste
	SRCS
		imgaste.cpp
		hostingservice.cpp
		requestbuilder.cpp
		uploader.cpp
	QT_COMPONENTS Network Widgets
	HAS_TESTS
	)

AddImgasteTest (upload tests/uploadtest)
