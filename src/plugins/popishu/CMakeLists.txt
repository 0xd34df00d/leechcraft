IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_popishu)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

SET (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
FIND_PACKAGE (QScintilla)
IF (NOT QSCINTILLA_FOUND)
	MESSAGE (ERROR "QScintilla not found, unable to build Popishu :(")
ENDIF (NOT QSCINTILLA_FOUND)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QSCINTILLA_INCLUDE_DIR}
	)
SET (SRCS
	popishu.cpp
	core.cpp
	editorpage.cpp
	xmlsettingsmanager.cpp
	replacedialog.cpp
	)
SET (HEADERS
	popishu.h
	core.h
	editorpage.h
	xmlsettingsmanager.h
	replacedialog.h
	)
SET (RESOURCES
	popishuresources.qrc
	)
SET (TRANSLATIONS
	leechcraft_popishu_ru_RU.ts
	leechcraft_popishu_uk_UA.ts
	leechcraft_popishu_en.ts
	leechcraft_popishu_de.ts
	leechcraft_popishu_fr.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_popishu_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_popishu_uk_UA.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_popishu_en.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_popishu_de.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_popishu_fr.qm
	)
SET (FORMS
	editorpage.ui
	replacedialog.ui
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
ADD_LIBRARY (leechcraft_popishu SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${HEADERS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_popishu
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QSCINTILLA_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_popishu DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES popishusettings.xml DESTINATION ${LC_SETTINGS_DEST})
