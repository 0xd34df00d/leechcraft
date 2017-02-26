include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (DCAC_SRCS
	dcac.cpp
	effectprocessor.cpp
	effects.cpp
	viewsmanager.cpp
	xmlsettingsmanager.cpp
	invertcolors.cpp
	reducelightness.cpp
	colortemp.cpp
	scriptobject.cpp
	scripthandler.cpp
	)
set (DCAC_RESOURCES
	)
CreateTrs ("poshuku_dcac" "en;ru_RU" DCAC_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku_dcac" "en;ru_RU" "${DCAC_SRCS}" "" "poshukudcacsettings.xml")

option (WITH_POSHUKU_DCAC_SIMD "Build SIMD-enabled versions of the effects (up to 6x performance improvements!)" ON)

if (NOT WITH_POSHUKU_DCAC_SIMD)
	add_definitions (-DPOSHUKU_DCAC_NO_SIMD)
endif ()

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

if (ENABLE_POSHUKU_DCAC_TESTS AND WITH_POSHUKU_DCAC_SIMD)
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
