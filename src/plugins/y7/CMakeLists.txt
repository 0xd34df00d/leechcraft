cmake_minimum_required (VERSION 3.3)
project (leechcraft_y7)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS y7.cpp
	QT_COMPONENTS Core
	)
