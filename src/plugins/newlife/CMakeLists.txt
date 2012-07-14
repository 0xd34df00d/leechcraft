IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_newlife)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

SET (QT_USE_QTXML TRUE)
SET (QT_USE_QTSQL TRUE)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

SET (SRCS
	newlife.cpp
	importwizard.cpp
	firstpage.cpp
	abstractimporter.cpp
	imhistimporterbase.cpp
	common/imimporter.cpp
	common/imimportpage.cpp
	common/xmlimaccount.cpp
	importers/akregator/akregatorimporter.cpp
	importers/akregator/akregatorimportpage.cpp
	importers/liferea/lifereaimportpage.cpp
	importers/liferea/lifereaimporter.cpp
	importers/kopete/kopeteimporter.cpp
	importers/kopete/kopeteimportpage.cpp
	importers/kopete/kopeteimportthread.cpp
	importers/ktorrent/ktorrentimportpage.cpp
	importers/ktorrent/ktorrentimporter.cpp
	importers/firefox/firefoximportpage.cpp
	importers/firefox/firefoximporter.cpp
	importers/firefox/firefoxprofileselectpage.cpp
	importers/psiplus/psiplusimporter.cpp
	importers/psiplus/psiplusimportpage.cpp
	importers/vacuum/vacuumimporter.cpp
	importers/vacuum/vacuumimportpage.cpp
	)
SET (HEADERS
	newlife.h
	importwizard.h
	firstpage.h
	abstractimporter.h
	imhistimporterbase.h
	common/imimporter.h
	common/imimportpage.h
	common/xmlimaccount.h
	importers/akregator/akregatorimporter.h
	importers/akregator/akregatorimportpage.h
	importers/liferea/lifereaimportpage.h
	importers/liferea/lifereaimporter.h
	importers/kopete/kopeteimporter.h
	importers/kopete/kopeteimportpage.h
	importers/kopete/kopeteimportthread.h
	importers/ktorrent/ktorrentimportpage.h
	importers/ktorrent/ktorrentimporter.h
	importers/firefox/firefoximportpage.h
	importers/firefox/firefoximporter.h
	importers/firefox/firefoxprofileselectpage.h
	importers/psiplus/psiplusimporter.h
	importers/psiplus/psiplusimportpage.h
	importers/vacuum/vacuumimporter.h
	importers/vacuum/vacuumimportpage.h
	)
SET (FORMS
	importwizard.ui
	feedssettingsimportpage.ui
	firstpage.ui
	common/imimportpage.ui
	importers/firefox/firefoxprofileselectpage.ui
	)
SET (RESOURCES newliferesources.qrc)
CreateTrVars ("newlife" "ar;be;de;en;fr;it;pl;ru_RU;uk_UA" TRANSLATIONS COMPILED_TRANSLATIONS)

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
ADD_LIBRARY (leechcraft_newlife SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_newlife
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_newlife DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
