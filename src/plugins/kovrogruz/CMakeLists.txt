cmake_minimum_required (VERSION 3.10)
project (kovrogruz)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS plugin.cpp
	QT_COMPONENTS Network
)
