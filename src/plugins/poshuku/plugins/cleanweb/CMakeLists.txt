project (poshuku_cleanweb)

LC_DEFINE_PLUGIN (
	SRCS
		cleanweb.cpp
		core.cpp
		subscriptionsmanagerwidget.cpp
		userfilters.cpp
		userfiltersmodel.cpp
		filter.cpp
		ruleoptiondialog.cpp
		wizardgenerator.cpp
		startupfirstpage.cpp
		subscriptionadddialog.cpp
		lineparser.cpp
		subscriptionsmodel.cpp
		pslhandler.cpp
		pslfetcher.cpp
	SETTINGS poshukucleanwebsettings.xml
	QT_COMPONENTS Concurrent Widgets Xml
	INSTALL_SHARE
	)

option (ENABLE_POSHUKU_CLEANWEB_TESTS "Build tests for Poshuku CleanWeb" ON)
if (ENABLE_POSHUKU_CLEANWEB_TESTS)
	function (AddCWTest _execName _cppFile _testName)
		set (_fullExecName lc_poshuku_cleanweb_${_execName}_test)
		add_executable (${_fullExecName} WIN32 ${_cppFile})
		target_link_libraries (${_fullExecName} ${LEECHCRAFT_LIBRARIES})
		add_test (${_testName} ${_fullExecName})
		FindQtLibs (${_fullExecName} Test)
	endfunction ()

	AddCWTest (pslhandler tests/pslhandlertest.cpp PoshukuCWPslHandlerTest)
endif ()
