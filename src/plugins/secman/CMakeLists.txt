CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_secman)
INCLUDE (InitLCPlugin OPTIONAL)

OPTION (TESTS_SECMAN "Enable SecMan tests")

IF (TESTS_SECMAN)
	SET (QT_USE_QTTEST TRUE)
ENDIF (TESTS_SECMAN)

INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

OPTION (ENABLE_SECMAN_SECURESTORAGE "Enable SecMan SecureStorage, the advanced encrypted private data storage" OFF)
OPTION (ENABLE_SECMAN_SIMPLESTORAGE "Enable SecMan SimpleStorage, the basic unencrypted private data storage" ON)

IF (ENABLE_SECMAN_SECURESTORAGE)
	ADD_SUBDIRECTORY (plugins/securestorage)
ENDIF (ENABLE_SECMAN_SECURESTORAGE)
IF (ENABLE_SECMAN_SIMPLESTORAGE)
	ADD_SUBDIRECTORY (plugins/simplestorage)
ENDIF (ENABLE_SECMAN_SIMPLESTORAGE)

SET (SRCS
	secman.cpp
	core.cpp
	contentsdisplaydialog.cpp
	)
SET (FORMS
	contentsdisplaydialog.ui
	)
SET (RESOURCES
	secmanresources.qrc
	)

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_secman SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)

FindQtLibs (leechcraft_secman Core)

TARGET_LINK_LIBRARIES (leechcraft_secman
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_secman DESTINATION ${LC_PLUGINS_DEST})
INSTALL (DIRECTORY interfaces DESTINATION "include/leechcraft")
