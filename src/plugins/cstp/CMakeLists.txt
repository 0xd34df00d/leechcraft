CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_cstp)
INCLUDE (InitLCPlugin OPTIONAL)

INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

SET (SRCS
	cstp.cpp
	core.cpp
	task.cpp
	addtask.cpp
	xmlsettingsmanager.cpp
	morphfile.cpp
	)
SET (HEADERS
	cstp.h
	core.h
	task.h
	addtask.h
	xmlsettingsmanager.h
	morphfile.h
	)
SET (FORMS
	addtask.ui
	)
SET (RESOURCES
	cstpresources.qrc
	)
CreateTrs ("cstp" "ar;de;en;eo;es;fr;it;oc;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_GUI_LIB)

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_cstp SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS})
LINK_DIRECTORIES (${Boost_LIBRARY_DIR})

FindQtLibs (leechcraft_cstp Network)

TARGET_LINK_LIBRARIES (leechcraft_cstp
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_cstp DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES cstpsettings.xml DESTINATION ${LC_SETTINGS_DEST})
