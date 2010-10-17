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
	akregatorimporter.cpp
	akregatorimportpage.cpp
	firstpage.cpp
	lifereaimportpage.cpp
	lifereaimporter.cpp
	ktorrentimportpage.cpp
	ktorrentimporter.cpp
	firefoximportpage.cpp
	firefoximporter.cpp
	)
SET (HEADERS
	newlife.h
	importwizard.h
	akregatorimporter.h
	akregatorimportpage.h
	firstpage.h
	lifereaimportpage.h
	lifereaimporter.h
	ktorrentimportpage.h
	ktorrentimporter.h
	abstractimporter.h
	firefoximportpage.h
	firefoximporter.h
	)
SET (FORMS
	importwizard.ui
	feedssettingsimportpage.ui
	firstpage.ui
	)
SET (TRANSLATIONS
	leechcraft_newlife_ru_RU.ts
	leechcraft_newlife_uk_UA.ts
	leechcraft_newlife_ar.ts
	leechcraft_newlife_be.ts
	leechcraft_newlife_de.ts
	leechcraft_newlife_en.ts
	leechcraft_newlife_pl.ts
	)
SET (RESOURCES newliferesources.qrc)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_newlife_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_newlife_uk_UA.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_newlife_ar.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_newlife_be.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_newlife_de.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_newlife_en.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_newlife_pl.qm
	)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
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
