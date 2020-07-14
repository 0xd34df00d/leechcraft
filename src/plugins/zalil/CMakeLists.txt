cmake_minimum_required (VERSION 3.3)
project (leechcraft_zalil)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		bitcheesepending.cpp
		bitcheeseservice.cpp
		pendinguploadbase.cpp
		servicebase.cpp
		servicesmanager.cpp
		zalil.cpp
	QT_COMPONENTS Network
	)
