cmake_minimum_required (VERSION 2.8)
project (leechcraft_poleemery)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (Qwt)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTSQL TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QWT_INCLUDE_DIRS}
	)
set (SRCS
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
set (FORMS
	accountstab.ui
	accountpropsdialog.ui
	operationstab.ui
	operationpropsdialog.ui
	graphstab.ui
	)
QT4_WRAP_UI (UIS_H ${FORMS})
CreateTrs ("poleemery" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poleemery" "en;ru_RU" "${SRCS}" "${FORMS}" "poleemerysettings.xml")

add_library (leechcraft_poleemery SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_poleemery
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QWT_LIBRARIES}
	)
install (TARGETS leechcraft_poleemery DESTINATION ${LC_PLUGINS_DEST})
install (FILES poleemerysettings.xml DESTINATION ${LC_SETTINGS_DEST})
