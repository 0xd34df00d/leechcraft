IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_dcminator)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED COMPONENTS date_time filesystem system thread signals)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

FIND_PACKAGE (BZip2 REQUIRED)
INCLUDE (./PCHSupport_26.cmake)
ADD_SUBDIRECTORY (dcpp)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/dcpp
	)
SET (SRCS
	dcminator.cpp
	core.cpp
	transferqueuemodel.cpp
	queueiteminfo.cpp
	hub.cpp
	userinfobase.cpp
	hubsortfiltermodel.cpp
	)
SET (HEADERS
	dcminator.h
	core.h
	transferqueuemodel.h
	queueiteminfo.h
	hub.h
	userinfobase.h
	hubsortfiltermodel.h
	)
SET (TRANSLATIONS
	leechcraft_dcminator_ru_RU.ts
	)
SET (FORMS
	hub.ui
	)
SET (RESOURCES resources.qrc)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_dcminator_ru_RU.qm
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
ADD_LIBRARY (leechcraft_dcminator SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_dcminator
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	dcpp
	)
INSTALL (TARGETS leechcraft_dcminator DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
