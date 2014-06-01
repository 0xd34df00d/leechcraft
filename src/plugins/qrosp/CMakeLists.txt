if (NOT QT_USE_FILE)
	cmake_minimum_required (VERSION 2.6)
	if (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	endif ()

	project (leechcraft_qrosp)

	if (NOT CMAKE_MODULE_PATH)
		set (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	endif ()

	find_package (Boost REQUIRED)
	find_package (Qt4 REQUIRED)
	find_package (LeechCraft REQUIRED)
endif ()

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package (Qross REQUIRED)
find_package (QJSON REQUIRED)

set (QT_USE_QTSCRIPT TRUE)
include (${QT_USE_FILE})
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
	wrappers/coreproxywrapper.cpp
	wrappers/hookproxywrapper.cpp
	wrappers/entitywrapper.cpp
	wrappers/shortcutproxywrapper.cpp
	wrappers/pluginsmanagerwrapper.cpp
	wrappers/tagsmanagerwrapper.cpp
	third-party/qmetaobjectbuilder_48.cpp
	)

add_library (leechcraft_qrosp SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_qrosp
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QROSS_LIBRARIES}
	${QJSON_LIBRARIES}
	)
install (TARGETS leechcraft_qrosp DESTINATION ${LC_PLUGINS_DEST})
