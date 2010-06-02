IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_qrosp)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

SET (QT_USE_QTSCRIPT TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	qrosp.cpp
	pluginmanager.cpp
	wrapperobject.cpp
	coreproxywrapper.cpp
	third-party/qmetaobjectbuilder.cpp
	)
SET (HEADERS
	qrosp.h
	pluginmanager.h
	wrapperobject.h
	coreproxywrapper.h
	third-party/qmetaobjectbuilder.h
	)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})

ADD_LIBRARY (leechcraft_qrosp SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_qrosp
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	qrosscore
	)
INSTALL (TARGETS leechcraft_qrosp DESTINATION ${LC_PLUGINS_DEST})
