set (CMAKE_INCLUDE_CURRENT_DIR ON)
find_package (Qt5Widgets)
find_package (Qt5LinguistTools REQUIRED)

set (CMAKE_AUTOMOC TRUE)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED TRUE)
set (CMAKE_CXX_EXTENSIONS OFF)

# Qt support helpers
macro (QtWrapUi outfiles)
	set (UIS_H)
	QT5_WRAP_UI (UIS_H ${ARGN})
	set (${outfiles} ${${outfiles}} ${UIS_H})
endmacro ()

macro (QtAddResources outfiles)
	set (RCCS)
	QT5_ADD_RESOURCES (RCCS ${ARGN} OPTIONS -compress-algo best -threshold 0)
	set (${outfiles} ${${outfiles}} ${RCCS})
endmacro ()

macro (FindQtLibs Target)
	cmake_policy (SET CMP0043 NEW)
	set (CMAKE_INCLUDE_CURRENT_DIR ON)

	set (_TARGET_QT_COMPONENTS "")
	foreach (V ${ARGN})
		if (NOT (${V} MATCHES ".*Private$"))
			list (APPEND _TARGET_QT_COMPONENTS ${V})
		endif ()
	endforeach ()
	find_package (Qt5 COMPONENTS ${_TARGET_QT_COMPONENTS})

	set (_TARGET_LINK_QT5_LIBS "")
	foreach (V ${ARGN})
		list (APPEND _TARGET_LINK_QT5_LIBS "Qt5::${V}")
	endforeach ()
	target_link_libraries (${Target} ${_TARGET_LINK_QT5_LIBS})
endmacro ()

# Plugin definition helpers
function (CreateTrs CompiledTranVar)
	file (GLOB TS_SOURCES "*.ts")
	if (TS_SOURCES)
		list (TRANSFORM TS_SOURCES REPLACE "(.*)\\.ts" "\\1.qm" OUTPUT_VARIABLE QM_RESULTS)
		add_custom_command (OUTPUT ${QM_RESULTS}
			COMMAND Qt5::lrelease ${TS_SOURCES}
			DEPENDS ${TS_SOURCES}
			)
		install (FILES ${QM_RESULTS} DESTINATION ${LC_TRANSLATIONS_DEST})
	else ()
		set (QM_RESULTS)
	endif ()
	set (${CompiledTranVar} ${QM_RESULTS} PARENT_SCOPE)
endfunction ()

function (LC_DEFINE_PLUGIN)
	set (options INSTALL_SHARE INSTALL_DESKTOP HAS_TESTS)
	set (one_value_args RESOURCES PLUGIN_VISIBLE_NAME)
	set (multi_value_args SRCS SETTINGS QT_COMPONENTS LINK_LIBRARIES)
	cmake_parse_arguments (P "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

	include_directories (
		${CMAKE_CURRENT_BINARY_DIR}
		${LEECHCRAFT_INCLUDE_DIR}
	)

	CreateTrs (QM_RESULTS)

	set (FULL_NAME leechcraft_${PROJECT_NAME})
	string (TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)

	add_library (${FULL_NAME} SHARED
		${QM_RESULTS}
		${P_SRCS}
		${P_RESOURCES}
		)
	set_target_properties (${FULL_NAME} PROPERTIES AUTOUIC TRUE AUTORCC TRUE)
	target_link_libraries (${FULL_NAME} ${LEECHCRAFT_LIBRARIES} ${P_LINK_LIBRARIES})

	if (P_PLUGIN_VISIBLE_NAME)
		target_compile_definitions (${FULL_NAME} PRIVATE PLUGIN_VISIBLE_NAME="${P_PLUGIN_VISIBLE_NAME}"_qs)
	endif ()

	install (TARGETS ${FULL_NAME} DESTINATION ${LC_PLUGINS_DEST})

	if (P_INSTALL_SHARE)
		install (DIRECTORY share/ DESTINATION ${LC_SHARE_DEST})
	endif ()

	if (P_SETTINGS)
		install (FILES ${P_SETTINGS} DESTINATION ${LC_SETTINGS_DEST})
	endif ()

	if (P_INSTALL_DESKTOP AND UNIX AND NOT APPLE)
		install (DIRECTORY freedesktop/ DESTINATION share/applications)
	endif ()

	if (P_HAS_TESTS)
		option (ENABLE_${PROJECT_NAME_UPPER}_TESTS "Enable ${PROJECT_NAME} tests" OFF)
		if (ENABLE_${PROJECT_NAME_UPPER}_TESTS)
			set (P_QT_COMPONENTS "${P_QT_COMPONENTS};Test")
		endif ()

		function (Add${PROJECT_NAME}Test _name _testObjectDef)
			string (TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)
			if (ENABLE_${PROJECT_NAME_UPPER}_TESTS)
				set (_fullExecName lc_${PROJECT_NAME}_test_${_name})

				set (_testRunner ${CMAKE_CURRENT_BINARY_DIR}/test_${_name}.cpp)
				file (WRITE ${_testRunner} "
#include <${_testObjectDef}.h>
#include <QtTest>

QTEST_APPLESS_MAIN (TheTestObject)
")

				add_executable (${_fullExecName} WIN32 ${_testRunner})

				add_test (${_fullExecName} ${_fullExecName})
				FindQtLibs (${_fullExecName} Test)
				target_link_libraries (${_fullExecName} leechcraft_${PROJECT_NAME} ${LEECHCRAFT_LIBRARIES})

				target_sources (leechcraft_${PROJECT_NAME} PRIVATE ${_testObjectDef}.cpp)
			endif ()
		endfunction ()
	endif ()

	FindQtLibs (${FULL_NAME} ${P_QT_COMPONENTS})
endfunction()

function (SUBPLUGIN suffix descr)
	if (${PROJECT_NAME} STREQUAL "leechcraft")
		set (prefix "")
	else ()
		string (TOUPPER ${PROJECT_NAME} prefix)
		set (prefix "${prefix}_")
	endif ()

	set (defVal "ON")
	if ("${ARGN}" STREQUAL "OFF")
		set (defVal ${ARGV2})
	endif ()
	string (TOLOWER ${suffix} suffixL)
	option (ENABLE_${prefix}${suffix} "${descr}" ${defVal})
	if (ENABLE_${prefix}${suffix})
		include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
		add_subdirectory (plugins/${suffixL})
	endif ()
endfunction ()
