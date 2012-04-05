CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_dolozhee)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	dolozhee.cpp
	xmlgenerator.cpp
	reportwizard.cpp
	chooseuserpage.cpp
	userstatuspage.cpp
	reporttypepage.cpp
	bugreportpage.cpp
	featurerequestpage.cpp
	finalpage.cpp
	)
SET (HEADERS
	dolozhee.h
	xmlgenerator.h
	reportwizard.h
	chooseuserpage.h
	userstatuspage.h
	reporttypepage.h
	featurerequestpage.h
	finalpage.h
	)
SET (FORMS
	chooseuserpage.ui
	userstatuspage.ui
	reporttypepage.ui
	bugreportpage.ui
	featurerequestpage.ui
	finalpage.ui
	)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (UIS_H ${FORMS})

ADD_LIBRARY (leechcraft_dolozhee SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_dolozhee
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_dolozhee DESTINATION ${LC_PLUGINS_DEST})
