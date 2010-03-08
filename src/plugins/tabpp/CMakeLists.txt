IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_tabpp)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)

	INCLUDE (${LEECHCRAFT_USE_FILE})
ENDIF (NOT QT_USE_FILE)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	tabpp.cpp
	core.cpp
	tabppwidget.cpp
	xmlsettingsmanager.cpp
	tabsfiltermodel.cpp
	)
SET (HEADERS
	tabpp.h
	core.h
	tabppwidget.h
	xmlsettingsmanager.h
	tabsfiltermodel.h
	)
SET (FORMS
	tabppwidget.ui
	)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})

ADD_LIBRARY (leechcraft_tabpp SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_tabpp
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_tabpp DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES tabppsettings.xml DESTINATION ${LC_SETTINGS_DEST})
