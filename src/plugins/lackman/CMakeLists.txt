cmake_minimum_required (VERSION 3.3)
project (leechcraft_lackman)
include (InitLCPlugin NO_POLICY_SCOPE)

option (TESTS_LACKMAN "Enable LackMan tests" OFF)

include_directories (SYSTEM
	${Boost_INCLUDE_DIR}
)
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
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
	)
set (RESOURCES
	lackmanresources.qrc
	)
set (FORMS
	lackmantab.ui
	)

CreateTrs ("lackman" "de;en;es;fr;it;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("lackman" "en;ru_RU" "${SRCS}" "${FORMS}" "lackmansettings.xml")

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_lackman SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_lackman
	${LEECHCRAFT_LIBRARIES}
	)

if (TESTS_LACKMAN)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests)
	add_executable (lc_lackman_versioncomparatortest WIN32
		tests/versioncomparatortest.cpp
		versioncomparator.cpp
		${VERSIONCOMPARATORTEST_MOC}
	)
	target_link_libraries (lc_lackman_versioncomparatortest
		${LEECHCRAFT_LIBRARIES}
	)

	FindQtLibs (lc_lackman_versioncomparatortest Test)

	add_test (VersionComparator lc_lackman_versioncomparatortest)
endif ()

install (TARGETS leechcraft_lackman DESTINATION ${LC_PLUGINS_DEST})
install (FILES lackmansettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_lackman Network Sql Widgets Xml XmlPatterns)
