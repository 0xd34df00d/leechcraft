cmake_minimum_required (VERSION 3.10)
project (qrosp)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (QrossCore REQUIRED IMPORTED_TARGET qrosscore)

LC_DEFINE_PLUGIN (
	SRCS
		qrosp.cpp
		pluginmanager.cpp
		wrapperobject.cpp
		typesfactory.cpp
		utilproxy.cpp
		scriptloaderinstance.cpp
		loadedscript.cpp
		lcenv.cpp
		wrappers/coreproxywrapper.cpp
		wrappers/hookproxywrapper.cpp
		wrappers/entitywrapper.cpp
		wrappers/shortcutproxywrapper.cpp
		wrappers/pluginsmanagerwrapper.cpp
		wrappers/tagsmanagerwrapper.cpp
	QT_COMPONENTS Script Widgets CorePrivate
	LINK_LIBRARIES PkgConfig::QrossCore
	)
