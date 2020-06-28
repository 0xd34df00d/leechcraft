project (leechcraft_poshuku_webengineview)

LC_DEFINE_PLUGIN (
	SRCS
		cookiessyncer.cpp
		customwebpage.cpp
		customwebview.cpp
		downloaditemhandler.cpp
		findnotification.cpp
		icondatabase.cpp
		icondatabaseondisk.cpp
		requestinterceptor.cpp
		webengineview.cpp
	QT_COMPONENTS PrintSupport Sql WebEngineWidgets
	)

option (ENABLE_POSHUKU_WEBENGINEVIEW_TESTS "Build tests for Poshuku WebEngineView" ON)
if (ENABLE_POSHUKU_WEBENGINEVIEW_TESTS)
	function (AddWEVTest _execName _cppFile _testName)
		set (_fullExecName lc_poshuku_dcac_${_execName}_test)
		add_executable (${_fullExecName} WIN32 ${_cppFile})
		target_link_libraries (${_fullExecName} ${LEECHCRAFT_LIBRARIES})
		add_test (${_testName} ${_fullExecName})
		FindQtLibs (${_fullExecName} Test WebEngineWidgets)
		add_dependencies (${_fullExecName} leechcraft_poshuku_webengineview)
	endfunction ()

	AddWEVTest (stringpathtrietest tests/stringpathtrietest.cpp PoshukuWEVStringPathTrieTest)
endif ()
