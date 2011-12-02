IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_liznoo)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
FIND_PACKAGE (Qwt6)

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

SET (QT_USE_QTDBUS TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${Qwt6_INCLUDE_DIR}
	)
SET (SRCS
	liznoo.cpp
	dbusconnector.cpp
	batteryinfo.cpp
	dbusthread.cpp
	batteryhistory.cpp
	batteryhistorydialog.cpp
	)
SET (HEADERS
	liznoo.h
	dbusconnector.h
	batteryinfo.h
	dbusthread.h
	batteryhistory.h
	batteryhistorydialog.h
	)
SET (FORMS
	batteryhistorydialog.ui
	)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (UIS_H ${FORMS})

ADD_LIBRARY (leechcraft_liznoo SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_liznoo
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${Qwt6_Qt4_LIBRARY}
	)
INSTALL (TARGETS leechcraft_liznoo DESTINATION ${LC_PLUGINS_DEST})
