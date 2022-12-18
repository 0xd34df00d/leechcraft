cmake_minimum_required (VERSION 3.10)
project (zalil)
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
