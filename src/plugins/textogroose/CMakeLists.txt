cmake_minimum_required (VERSION 3.10)
project (leechcraft_textogroose)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		textogroose.cpp
		apiobject.cpp
	QT_COMPONENTS Core
	)
