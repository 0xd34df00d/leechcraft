include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (WEBENGINEVIEW_SRCS
	cookiessyncer.cpp
	customwebpage.cpp
	customwebview.cpp
	downloaditemhandler.cpp
	findnotification.cpp
	icondatabase.cpp
	icondatabaseondisk.cpp
	requestinterceptor.cpp
	webengineview.cpp
	)
set (WEBENGINEVIEW_FORMS
	)
set (WEBENGINEVIEW_RESOURCES)
#CreateTrs ("poshuku_webengineview" "en;ru_RU" WEBENGINEVIEW_COMPILED_TRANSLATIONS)
#CreateTrsUpTarget("poshuku_webengineview" "en;ru_RU" "${WEBENGINEVIEW_SRCS}" "${WEBENGINEVIEW_FORMS}" "poshukuwebengineviewsettings.xml")

QtWrapUi (WEBENGINEVIEW_UIS_H ${WEBENGINEVIEW_FORMS})
QtAddResources (WEBENGINEVIEW_RCCS ${WEBENGINEVIEW_RESOURCES})
add_library (leechcraft_poshuku_webengineview SHARED
	${WEBENGINEVIEW_COMPILED_TRANSLATIONS}
	${WEBENGINEVIEW_SRCS}
	${WEBENGINEVIEW_UIS_H}
	${WEBENGINEVIEW_RCCS}
	)
target_link_libraries (leechcraft_poshuku_webengineview
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_webengineview DESTINATION ${LC_PLUGINS_DEST})
#install (FILES poshukuwebengineviewsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_poshuku_webengineview PrintSupport Sql WebEngineWidgets)

option (ENABLE_POSHUKU_WEBENGINEVIEW_TESTS "Build tests for Poshuku WebEngineView" ON)

if (ENABLE_POSHUKU_WEBENGINEVIEW_TESTS)
	function (AddWEVTest _execName _cppFile _testName)
		set (_fullExecName lc_poshuku_dcac_${_execName}_test)
		add_executable (${_fullExecName} WIN32 ${_cppFile})
		target_link_libraries (${_fullExecName} ${LEECHCRAFT_LIBRARIES})
		add_test (${_testName} ${_fullExecName})
		FindQtLibs (${_fullExecName} Test WebEngineWidgets)
		add_dependencies (${_fullExecName} leechcraft_poshuku_dcac)
	endfunction ()

	AddWEVTest (stringpathtrietest tests/stringpathtrietest.cpp PoshukuWEVStringPathTrieTest)
endif ()
