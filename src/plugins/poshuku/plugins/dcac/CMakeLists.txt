project (poshuku_dcac)

LC_DEFINE_PLUGIN (
	SRCS
		dcac.cpp
		effectprocessor.cpp
		effects.cpp
		viewsmanager.cpp
		invertcolors.cpp
		reducelightness.cpp
		colortemp.cpp
		scriptobject.cpp
		scripthandler.cpp
	SETTINGS poshukudcacsettings.xml
	QT_COMPONENTS Widgets WebKitWidgets
	)

option (ENABLE_POSHUKU_DCAC_TESTS "Build tests for Poshuku DCAC" ON)
if (ENABLE_POSHUKU_DCAC_TESTS)
	function (AddDCACTest _execName _cppFile _testName)
		set (_fullExecName lc_poshuku_dcac_${_execName}_test)
		add_executable (${_fullExecName} WIN32 ${_cppFile} tests/testbase.cpp)
		target_link_libraries (${_fullExecName} ${LEECHCRAFT_LIBRARIES})
		add_test (${_testName} ${_fullExecName})
		FindQtLibs (${_fullExecName} Concurrent Gui Test)
		add_dependencies (${_fullExecName} leechcraft_poshuku_dcac)
	endfunction ()

	AddDCACTest (getgray tests/getgraytest.cpp PoshukuDCACGetGrayTest)
	AddDCACTest (reducelightness tests/reducelightnesstest.cpp PoshukuDCACReduceLightnessTest)
	AddDCACTest (invertrgb tests/invertrgbtest.cpp PoshukuDCACInvertRgbTest)
	AddDCACTest (temp2rgb tests/temp2rgbtest.cpp PoshukuDCACTemp2RgbTest)
	AddDCACTest (colortemptest tests/colortemptest.cpp PoshukuDCACColorTempTest)
endif ()
