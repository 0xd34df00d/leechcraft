set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

option (WITH_X11 "Enable X11 support on Linux" ON)
option (WITH_QWT "Enable support for Qwt (for QML PlotItem, for example)" ON)

include_directories (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	..)

set (LC_SOVERSION 0.6.70)

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

if (WITH_X11 AND NOT APPLE AND NOT USE_QT5)
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
	leechcraft-util-xdg
	leechcraft-util-xpc
	leechcraft-util-xsd
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

add_library (leechcraft-util SHARED
	${SRCS}
	)
target_link_libraries (leechcraft-util
	${QT_LIBRARIES}
	${ADDITIONAL_LIBS}
	)
set_property (TARGET leechcraft-util PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util Xml Qml Sql Widgets)
