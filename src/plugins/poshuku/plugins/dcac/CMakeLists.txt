include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (DCAC_SRCS
	dcac.cpp
	effectprocessor.cpp
	effects.cpp
	viewsmanager.cpp
	xmlsettingsmanager.cpp
	effectsimpl.cpp
	)
set (DCAC_RESOURCES
	)
CreateTrs ("poshuku_dcac" "en;ru_RU" DCAC_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku_dcac" "en;ru_RU" "${DCAC_SRCS}" "" "poshukudcacsettings.xml")

QtAddResources (DCAC_RCCS ${DCAC_RESOURCES})
add_library (leechcraft_poshuku_dcac SHARED
	${DCAC_COMPILED_TRANSLATIONS}
	${DCAC_SRCS}
	${DCAC_MOC_SRCS}
	${DCAC_RCCS}
	)
target_link_libraries (leechcraft_poshuku_dcac
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_dcac DESTINATION ${LC_PLUGINS_DEST})
install (FILES poshukudcacsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_poshuku_dcac Widgets WebKitWidgets)

option (ENABLE_POSHUKU_DCAC_TESTS "Build tests for Poshuku DCAC" ON)

if (ENABLE_POSHUKU_DCAC_TESTS)
	function (AddDCACTest _execName _cppFile _testName)
		set (_fullExecName lc_poshuku_dcac_${_execName}_test)
		add_executable (${_fullExecName} WIN32 ${_cppFile} tests/testbase.cpp)
		target_link_libraries (${_fullExecName} ${LEECHCRAFT_LIBRARIES})
		add_test (${_testName} ${_fullExecName})
		FindQtLibs (${_fullExecName} Test)
		add_dependencies (${_fullExecName} leechcraft_poshuku_dcac)
	endfunction ()

	AddDCACTest (getgray tests/getgraytest.cpp PoshukuDCACGetGrayTest)
endif ()
