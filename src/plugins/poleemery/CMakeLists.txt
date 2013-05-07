CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_poleemery)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTSQL TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
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
	)
SET (FORMS
	accountstab.ui
	accountpropsdialog.ui
	operationstab.ui
	operationpropsdialog.ui
	)
QT4_WRAP_UI (UIS_H ${FORMS})
#CreateTrs("poleemery" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poleemery" "en;ru_RU" "${SRCS}" "${FORMS}" "")

ADD_LIBRARY (leechcraft_poleemery SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_poleemery
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_poleemery DESTINATION ${LC_PLUGINS_DEST})
