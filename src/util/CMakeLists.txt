set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

option (ENABLE_UTIL_TESTS "Enable tests for some of the Utility components" OFF)
option (WITH_X11 "Enable X11 support on Linux" ON)
option (WITH_QWT "Enable support for Qwt (for QML PlotItem, for example)" ON)

include_directories (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	..)

if (ENABLE_UTIL_TESTS)
	function (AddUtilTest _execName _cppFile _testName _depLib)
		set (_fullExecName lc_util_${_execName}_test)
		add_executable (${_fullExecName} WIN32 ${_cppFile})
		add_test (${_testName} ${_fullExecName})
		FindQtLibs (${_fullExecName} Test)
		add_dependencies (${_fullExecName} ${_depLib})
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
add_subdirectory (xdg)
add_subdirectory (xpc)
add_subdirectory (xsd)

if (WITH_X11 AND NOT APPLE)
	add_subdirectory (x11)
	set (LEECHCRAFT_UTIL_LIBRARIES leechcraft-util-x11${LC_LIBSUFFIX})
endif ()

set (LEECHCRAFT_UTIL_LIBRARIES
	${LEECHCRAFT_UTIL_LIBRARIES}
	leechcraft-util${LC_LIBSUFFIX}
	leechcraft-util-db${LC_LIBSUFFIX}
	leechcraft-util-gui${LC_LIBSUFFIX}
	leechcraft-util-models${LC_LIBSUFFIX}
	leechcraft-util-network${LC_LIBSUFFIX}
	leechcraft-util-qml${LC_LIBSUFFIX}
	leechcraft-util-shortcuts${LC_LIBSUFFIX}
	leechcraft-util-sll${LC_LIBSUFFIX}
	leechcraft-util-svcauth${LC_LIBSUFFIX}
	leechcraft-util-sys${LC_LIBSUFFIX}
	leechcraft-util-tags${LC_LIBSUFFIX}
	leechcraft-util-xdg${LC_LIBSUFFIX}
	leechcraft-util-xpc${LC_LIBSUFFIX}
	leechcraft-util-xsd${LC_LIBSUFFIX}
	)

set (LEECHCRAFT_UTIL_LIBRARIES ${LEECHCRAFT_UTIL_LIBRARIES} PARENT_SCOPE)

set (SRCS
	util.cpp
	structuresops.cpp
	exceptions.cpp
	)

foreach (SRC ${SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/${SRC}")
endforeach ()
foreach (FORM ${FORMS})
	set (UTIL_FORMS "${UTIL_FORMS};util/${FORM}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)
set (UTIL_FORMS ${UTIL_FORMS} PARENT_SCOPE)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_NO_DEBUG)
add_definitions(-DQT_SHARED)
add_definitions(-DQT_GUI_LIB)

add_library (leechcraft-util${LC_LIBSUFFIX} SHARED
	${SRCS}
	)
target_link_libraries (leechcraft-util${LC_LIBSUFFIX}
	${QT_LIBRARIES}
	${ADDITIONAL_LIBS}
	)
set_property (TARGET leechcraft-util${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util${LC_LIBSUFFIX} Xml Qml Sql Widgets)
