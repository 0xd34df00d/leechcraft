if (NOT QT_USE_FILE)
	cmake_minimum_required (VERSION 2.6)
	if (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	endif (COMMAND cmake_policy)

	project (leechcraft_qrosp)

	if (NOT CMAKE_MODULE_PATH)
		set (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	endif (NOT CMAKE_MODULE_PATH)

	find_package (Boost REQUIRED)
	find_package (Qt4 REQUIRED)
	find_package (LeechCraft REQUIRED)
endif (NOT QT_USE_FILE)

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
	)
set (HEADERS
	qrosp.h
	pluginmanager.h
	wrapperobject.h
	typesfactory.h
	utilproxy.h
	scriptloaderinstance.h
	loadedscript.h
	wrappers/coreproxywrapper.h
	wrappers/hookproxywrapper.h
	wrappers/entitywrapper.h
	wrappers/shortcutproxywrapper.h
	wrappers/pluginsmanagerwrapper.h
	wrappers/tagsmanagerwrapper.h
	)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif (NOT LC_NO_MOC)

if (${QT_VERSION_MINOR} LESS 8)
	set (SRCS ${SRCS} third-party/qmetaobjectbuilder.cpp)
	set (HEADERS ${HEADERS} third-party/qmetaobjectbuilder.h)
elseif (${QT_VERSION_MINOR} EQUAL 8)
	set (SRCS ${SRCS} third-party/qmetaobjectbuilder_48.cpp)
	set (HEADERS ${HEADERS} third-party/qmetaobjectbuilder_48.h)
else (${QT_VERSION_MINOR} LESS 8)
	message (ERROR "Unsupported Qt version ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}, Qrosp cannot be built")
endif (${QT_VERSION_MINOR} LESS 8)

add_library (leechcraft_qrosp SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
target_link_libraries (leechcraft_qrosp
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QROSS_LIBRARIES}
	${QJSON_LIBRARIES}
	)
install (TARGETS leechcraft_qrosp DESTINATION ${LC_PLUGINS_DEST})
