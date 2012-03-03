IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_snails)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

FIND_PACKAGE (MAGIC)
IF (MAGIC_FOUND)
	ADD_DEFINITIONS (-DHAVE_MAGIC)
ENDIF (MAGIC_FOUND)

SET (QT_USE_QTWEBKIT TRUE)
SET (QT_USE_QTSQL TRUE)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${MAGIC_INCLUDE_DIRS}
	)
SET (SRCS
	snails.cpp
	core.cpp
	mailtab.cpp
	xmlsettingsmanager.cpp
	accountslistwidget.cpp
	account.cpp
	accountconfigdialog.cpp
	message.cpp
	accountthread.cpp
	accountthreadworker.cpp
	progresslistener.cpp
	storage.cpp
	progressmanager.cpp
	mailtreedelegate.cpp
	composemessagetab.cpp
	accountfoldermanager.cpp
	attdescr.cpp
	vmimeconversions.cpp
	outputiodevadapter.cpp
	mailmodelmanager.cpp
	common.cpp
	)
SET (HEADERS
	snails.h
	core.h
	mailtab.h
	xmlsettingsmanager.h
	accountslistwidget.h
	account.h
	accountconfigdialog.h
	message.h
	accountthread.h
	accountthreadworker.h
	progresslistener.h
	storage.h
	progressmanager.h
	mailtreedelegate.h
	composemessagetab.h
	accountfoldermanager.h
	attdescr.h
	vmimeconversions.h
	outputiodevadapter.h
	mailmodelmanager.h
	common.h
	)
SET (FORMS
	mailtab.ui
	accountslistwidget.ui
	accountconfigdialog.ui
	composemessagetab.ui
	)
SET (RESOURCES snailsresources.qrc)
CreateTrVars ("snails" "en;ru_RU" TRANSLATIONS COMPILED_TRANSLATIONS)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_snails SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_snails
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${MAGIC_LIBRARIES}
	vmime
	)
INSTALL (TARGETS leechcraft_snails DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES snailssettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (DIRECTORY share/snails DESTINATION share/leechcraft)
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
