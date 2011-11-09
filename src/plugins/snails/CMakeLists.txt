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

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

SET (QT_USE_QTWEBKIT TRUE)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
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
	)
SET (FORMS
	mailtab.ui
	accountslistwidget.ui
	accountconfigdialog.ui
	)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})

ADD_LIBRARY (leechcraft_snails SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_snails
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	vmime
	)
INSTALL (TARGETS leechcraft_snails DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES snailssettings.xml DESTINATION ${LC_SETTINGS_DEST})
