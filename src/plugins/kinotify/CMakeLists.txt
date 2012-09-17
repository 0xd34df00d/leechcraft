CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_kinotify)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTWEBKIT TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	kinotify.cpp
	kinotifywidget.cpp
	notificationaction.cpp
	xmlsettingsmanager.cpp
	)

IF (UNIX AND NOT APPLE)
	SET (SRCS fswinwatcher_x11.cpp ${SRCS})
ELSEIF (WIN32)
	SET (SRCS fswinwatcher_win.cpp ${SRCS})
ELSE (UNIX AND NOT APPLE)
	SET (SRCS fswinwatcher.cpp ${SRCS})
ENDIF (UNIX AND NOT APPLE)

SET (HEADERS
	fswinwatcher.h
	kinotify.h
	kinotifywidget.h
	notificationaction.h
	xmlsettingsmanager.h
	)
SET (RESOURCES kinotifyresources.qrc)
CreateTrs ("kinotify" "ar;de;en;es;fr;it;ru_RU;uk_UA" COMPILED_TRANSLATIONS)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_kinotify SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_kinotify
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_kinotify DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES kinotifysettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (DIRECTORY themes DESTINATION ${LC_SHARE_DEST}/kinotify)
