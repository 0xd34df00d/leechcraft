cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_mucommands)
include (InitLCPlugin OPTIONAL)

option (ENABLE_AZOTH_MUCOMMANDS_TESTS "Enable tests for Azoth MuCommands" OFF)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

set (MUCOMMANDS_SRCS
	mucommands.cpp
	commands.cpp
	presencecommand.cpp
	openurlcommand.cpp
	descparser.cpp
	)

set (MUCOMMANDS_RESOURCES azothmucommandsresources.qrc)
QtAddResources (MUCOMMANDS_RCCS ${MUCOMMANDS_RESOURCES})

add_custom_target (leechcraft_azoth_mucommands_gen_descr_ts
		"${CMAKE_CURRENT_SOURCE_DIR}/descr_transform.sh"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" VERBATIM)

CreateTrs ("azoth_mucommands" "en;ru_RU" MUCOMMANDS_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_mucommands" "en;ru_RU" "${MUCOMMANDS_SRCS}" "" ""
		DEPENDS leechcraft_azoth_mucommands_gen_descr_ts)

add_library (leechcraft_azoth_mucommands SHARED
	${MUCOMMANDS_COMPILED_TRANSLATIONS}
	${MUCOMMANDS_RCCS}
	${MUCOMMANDS_SRCS}
	)
target_link_libraries (leechcraft_azoth_mucommands
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_mucommands DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_azoth_mucommands Core Xml Widgets)

if (ENABLE_AZOTH_MUCOMMANDS_TESTS)
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR})

	function (AddMuCTest _execName _cppFile _testName)
		set (_fullExecName lc_azoth_mucommands_${_execName}_test)
		add_executable (${_fullExecName} WIN32 ${_cppFile})
		target_link_libraries (${_fullExecName} ${LEECHCRAFT_LIBRARIES})
		add_test (${_testName} ${_fullExecName})
		FindQtLibs (${_fullExecName} Test)
		add_dependencies (${_fullExecName} leechcraft_azoth_mucommands)
	endfunction ()

	AddMuCTest (presence tests/presencecommandtest.cpp AzothMuCommandsPresenceTest)
	AddMuCTest (openurl tests/openurlcommandtest.cpp AzothMuCommandsOpenUrlTest)
endif ()
