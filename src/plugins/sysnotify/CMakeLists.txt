CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_sysnotify)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTDBUS TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	sysnotify.cpp
	notificationmanager.cpp
	)
#CreateTrs("sysnotify" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("sysnotify" "en;ru_RU" "${SRCS}" "${FORMS}" "")

ADD_LIBRARY (leechcraft_sysnotify SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_sysnotify
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_sysnotify DESTINATION ${LC_PLUGINS_DEST})
