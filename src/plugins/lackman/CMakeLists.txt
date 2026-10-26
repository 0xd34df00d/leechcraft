cmake_minimum_required (VERSION 3.10)
project (lackman)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		lackman.cpp
		lackmantab.cpp
		core.cpp
		repoinfo.cpp
		repoinfofetcher.cpp
		storage.cpp
		deptreebuilder.cpp
		packagesmodel.cpp
		xmlparsers.cpp
		externalresourcemanager.cpp
		pendingmanager.cpp
		packageprocessor.cpp
		versioncomparator.cpp
		typefilterproxymodel.cpp
		stringfiltermodel.cpp
		updatesnotificationmanager.cpp
		lackmanutil.cpp
	RESOURCES lackmanresources.qrc
	SETTINGS lackmansettings.xml
	QT_COMPONENTS Network Sql Widgets Xml
	INSTALL_SHARE
	HAS_TESTS
	)

AddLackmanTest (versioncomparator SRCS tests/versioncomparatortest.cpp versioncomparator.cpp)
