cmake_minimum_required (VERSION 2.8)
project (leechcraft_lackman)
include (InitLCPlugin OPTIONAL)

option (TESTS_LACKMAN "Enable LackMan tests" OFF)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	lackman.cpp
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
	)
CreateTrs ("lackman" "de;en;es;fr;it;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
set (RESOURCES
	lackmanresources.qrc
	)
set (FORMS
	lackman.ui
	)

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_lackman SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)

FindQtLibs (leechcraft_lackman Network Xml XmlPatterns Sql)

target_link_libraries (leechcraft_lackman
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

if (TESTS_LACKMAN)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests)
	QT4_WRAP_CPP (VERSIONCOMPARATORTEST_MOC "tests/versioncomparatortest.h")
	add_executable (lc_lackman_versioncomparatortest WIN32
		tests/versioncomparatortest.cpp
		versioncomparator.cpp
		${VERSIONCOMPARATORTEST_MOC}
	)
	target_link_libraries (lc_lackman_versioncomparatortest
		${QT_LIBRARIES}
		${LEECHCRAFT_LIBRARIES}
	)

	add_test (VersionComparator lc_lackman_versioncomparatortest)
endif ()

install (TARGETS leechcraft_lackman DESTINATION ${LC_PLUGINS_DEST})
install (FILES lackmansettings.xml DESTINATION ${LC_SETTINGS_DEST})
