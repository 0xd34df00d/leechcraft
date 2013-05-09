CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_poleemery)
INCLUDE (InitLCPlugin OPTIONAL)

SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
FIND_PACKAGE (Qwt)

SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTSQL TRUE)
SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QWT_INCLUDE_DIRS}
	)
SET (SRCS
	poleemery.cpp
	core.cpp
	structures.cpp
	storage.cpp
	accountstab.cpp
	accountpropsdialog.cpp
	accountsmanager.cpp
	operationstab.cpp
	operationsmanager.cpp
	operationpropsdialog.cpp
	entriesmodel.cpp
	currenciesmanager.cpp
	xmlsettingsmanager.cpp
	graphstab.cpp
	graphsfactory.cpp
	)
SET (FORMS
	accountstab.ui
	accountpropsdialog.ui
	operationstab.ui
	operationpropsdialog.ui
	graphstab.ui
	)
QT4_WRAP_UI (UIS_H ${FORMS})
CreateTrs ("poleemery" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poleemery" "en;ru_RU" "${SRCS}" "${FORMS}" "poleemerysettings.xml")

ADD_LIBRARY (leechcraft_poleemery SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_poleemery
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QWT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_poleemery DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES poleemerysettings.xml DESTINATION ${LC_SETTINGS_DEST})
