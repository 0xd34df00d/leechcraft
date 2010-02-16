IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_kinotify)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

SET (QT_USE_QTWEBKIT TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	kinotify.cpp
	kineticnotification.cpp
	notificationwidget.cpp
	notificationsmanager.cpp
	)
SET (HEADERS
	kinotify.h
	kineticnotification.h
	notificationwidget.h
	notificationsmanager.h
	)
SET (RESOURCES kinotifyresources.qrc)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_kinotify SHARED
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_kinotify
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_kinotify DESTINATION ${LC_PLUGINS_DEST})
