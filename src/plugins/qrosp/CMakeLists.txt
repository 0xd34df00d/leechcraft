cmake_minimum_required (VERSION 2.8)
project (leechcraft_qrosp)
include (InitLCPlugin OPTIONAL)

if (NOT USE_QT5)
	find_package (QJSON REQUIRED)
else ()
	unset (QJSON_INCLUDE_DIR)
	unset (QJSON_LIBRARIES)
endif ()

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package (Qross REQUIRED)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QROSS_INCLUDE_DIR}
	${QJSON_INCLUDE_DIR}
	)
set (SRCS
	qrosp.cpp
	pluginmanager.cpp
	wrapperobject.cpp
	typesfactory.cpp
	utilproxy.cpp
	scriptloaderinstance.cpp
	loadedscript.cpp
	lcenv.cpp
	wrappers/coreproxywrapper.cpp
	wrappers/hookproxywrapper.cpp
	wrappers/entitywrapper.cpp
	wrappers/shortcutproxywrapper.cpp
	wrappers/pluginsmanagerwrapper.cpp
	wrappers/tagsmanagerwrapper.cpp
	)

if (NOT USE_QT5)
	set (SRCS
		${SRCS}
		third-party/qmetaobjectbuilder_48.cpp
		)
endif ()

add_library (leechcraft_qrosp SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_qrosp
	${LEECHCRAFT_LIBRARIES}
	${QROSS_LIBRARIES}
	${QJSON_LIBRARIES}
	)
install (TARGETS leechcraft_qrosp DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_qrosp Script Widgets)

include_directories (${Qt5Core_PRIVATE_INCLUDE_DIRS})
