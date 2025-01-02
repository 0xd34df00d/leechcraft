set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

option (ENABLE_UTIL_TESTS "Enable tests for some of the Utility components" OFF)
option (WITH_X11 "Enable X11 support on Linux" ON)
option (WITH_QWT "Enable support for Qwt (for QML PlotItem, for example)" ON)

include_directories (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
	..)

if (ENABLE_UTIL_TESTS)
	function (AddUtilTest _execName _cppFile _testName _depLib)
		set (_fullExecName lc_util_${_execName}_test)
		add_executable (${_fullExecName} WIN32 ${_cppFile})
		add_test (${_testName} ${_fullExecName})
		FindQtLibs (${_fullExecName} Test)
		target_link_libraries (${_fullExecName} ${_depLib})
	endfunction ()
endif ()

add_subdirectory (db)
add_subdirectory (gui)
add_subdirectory (models)
add_subdirectory (network)
add_subdirectory (qml)
add_subdirectory (shortcuts)
add_subdirectory (sll)
add_subdirectory (svcauth)
add_subdirectory (sys)
add_subdirectory (tags)
add_subdirectory (threads)
add_subdirectory (xdg)
add_subdirectory (xpc)
add_subdirectory (xsd)

if (WITH_X11 AND NOT APPLE)
	add_subdirectory (x11)
	set (LEECHCRAFT_UTIL_LIBRARIES leechcraft-util-x11)
endif ()

set (LEECHCRAFT_UTIL_LIBRARIES
	${LEECHCRAFT_UTIL_LIBRARIES}
	leechcraft-util
	leechcraft-util-db
	leechcraft-util-gui
	leechcraft-util-models
	leechcraft-util-network
	leechcraft-util-qml
	leechcraft-util-shortcuts
	leechcraft-util-sll
	leechcraft-util-svcauth
	leechcraft-util-sys
	leechcraft-util-tags
	leechcraft-util-threads
	leechcraft-util-xdg
	leechcraft-util-xpc
	leechcraft-util-xsd
	)

set (LEECHCRAFT_UTIL_LIBRARIES ${LEECHCRAFT_UTIL_LIBRARIES} PARENT_SCOPE)

add_util_library (util
	SOURCES
		util.cpp
		structuresops.cpp
	USES Widgets
	)
