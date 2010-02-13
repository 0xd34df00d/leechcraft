IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_remoter)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

SET (QT_USE_QTXML TRUE)
SET (QT_USE_QTNETWORK TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${LEECHCRAFT_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/webclient/src/
	${CMAKE_CURRENT_SOURCE_DIR}/webclient/3rdparty/json/
	)

SET (SRCS
    remoter.cpp
	core.cpp
	webclient/src/widgeteventhandler.cpp
	webclient/src/eventqueue.cpp
	webclient/src/webclientserver.cpp
	webclient/src/webclient.cpp
	webclient/src/sessionserver.cpp
	webclient/3rdparty/json/arraylist.c
	webclient/3rdparty/json/debug.c
	webclient/3rdparty/json/json_object.c
	webclient/3rdparty/json/json_tokener.c
	webclient/3rdparty/json/json_util.c
	webclient/3rdparty/json/linkhash.c
	webclient/3rdparty/json/printbuf.c
	)
SET (HEADERS
    remoter.h
	core.h
	webclient/src/webclient.h
	webclient/src/widgeteventhandler.h
	webclient/src/eventqueue.h
	webclient/src/webclientserver.h
	webclient/src/sessionserver.h
	webclient/3rdparty/json/arraylist.h
	webclient/3rdparty/json/config.h
	webclient/3rdparty/json/json.h
	webclient/3rdparty/json/json_object_private.h
	webclient/3rdparty/json/json_util.h
	webclient/3rdparty/json/printbuf.h
	webclient/3rdparty/json/bits.h
	webclient/3rdparty/json/debug.h
	webclient/3rdparty/json/json_object.h
	webclient/3rdparty/json/json_tokener.h
	webclient/3rdparty/json/linkhash.h
	)
SET (RESOURCES resources.qrc
	webclient/src/src.qrc
	webclient/3rdparty/dojo.qrc
	)
SET (TRANSLATIONS
	leechcraft_remoter_ru_RU.ts
	leechcraft_remoter_uk_UA.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_remoter_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_remoter_uk_UA.qm
	)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_remoter SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS})
TARGET_LINK_LIBRARIES (leechcraft_remoter
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_remoter DESTINATION ${LC_PLUGINS_DEST})
