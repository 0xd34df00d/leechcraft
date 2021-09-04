cmake_minimum_required (VERSION 3.10)
project (leechcraft_lackman)
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
		packagesdelegate.cpp
		xmlparsers.cpp
		externalresourcemanager.cpp
		pendingmanager.cpp
		packageprocessor.cpp
		versioncomparator.cpp
		typefilterproxymodel.cpp
		xmlsettingsmanager.cpp
		delegatebuttongroup.cpp
		stringfiltermodel.cpp
		updatesnotificationmanager.cpp
		lackmanutil.cpp
	RESOURCES lackmanresources.qrc
	SETTINGS lackmansettings.xml
	QT_COMPONENTS Network Sql Widgets Xml XmlPatterns
	INSTALL_SHARE
	)

option (TESTS_LACKMAN "Enable LackMan tests" OFF)
if (TESTS_LACKMAN)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests)
	add_executable (lc_lackman_versioncomparatortest WIN32
		tests/versioncomparatortest.cpp
		versioncomparator.cpp
	)
	target_link_libraries (lc_lackman_versioncomparatortest
		${LEECHCRAFT_LIBRARIES}
	)

	FindQtLibs (lc_lackman_versioncomparatortest Test)

	add_test (VersionComparator lc_lackman_versioncomparatortest)
endif ()
