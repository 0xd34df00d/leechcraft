cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_mucommands)
include (InitLCPlugin NO_POLICY_SCOPE)

option (ENABLE_AZOTH_MUCOMMANDS_TESTS "Enable tests for Azoth MuCommands" OFF)

LC_DEFINE_PLUGIN (azoth_mucommands
	SRCS
		mucommands.cpp
		commands.cpp
		presencecommand.cpp
		openurlcommand.cpp
		descparser.cpp
	QT_COMPONENTS Widgets
	)

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
