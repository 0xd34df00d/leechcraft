IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_azoth)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

SET (QT_USE_QTDBUS TRUE)
SET (QT_USE_QTWEBKIT TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	azoth.cpp
	core.cpp
	mainwidget.cpp
	chattabsmanager.cpp
	chattab.cpp
	sortfilterproxymodel.cpp
	)
SET (HEADERS
	azoth.h
	core.h
	mainwidget.h
	chattabsmanager.h
	chattab.h
	sortfilterproxymodel.h
	interfaces/iprotocolplugin.h
	interfaces/iaccount.h
	interfaces/iprotocol.h
	interfaces/iclentry.h
	)
SET (FORMS
	mainwidget.ui
	chattab.ui
	)
SET (RESOURCES azothresources.qrc)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_azoth SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_azoth DESTINATION ${LC_PLUGINS_DEST})

SET (AZOTH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
ADD_SUBDIRECTORY (plugins/xoox)
