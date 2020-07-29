cmake_minimum_required (VERSION 3.10)
project (leechcraft_lmp_ppl)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${LMP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

LC_DEFINE_PLUGIN (
	SRCS
		loghandler.cpp
		parser.cpp
		ppl.cpp
		tracksselectordialog.cpp
	QT_COMPONENTS Widgets
	)

option (ENABLE_LMP_PPL_TESTS "Build tests for LMP PPL" ON)
if (ENABLE_LMP_PPL_TESTS)
	function (AddPPLTest _execName _cppFile _testName)
		set (_fullExecName lc_lmp_ppl_${_execName}_test)
		add_executable (${_fullExecName} WIN32 ${_cppFile})
		target_link_libraries (${_fullExecName} ${LEECHCRAFT_LIBRARIES})
		add_test (${_testName} ${_fullExecName})
		FindQtLibs (${_fullExecName} Concurrent Test)
		add_dependencies (${_fullExecName} leechcraft_lmp_ppl)
	endfunction ()

	AddPPLTest (parser tests/parsertest.cpp LMPPPLParserTest)
endif ()
