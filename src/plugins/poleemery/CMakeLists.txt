cmake_minimum_required (VERSION 3.3)
project (leechcraft_poleemery)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (Qwt)

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
	entriesdelegate.cpp
	)
set (FORMS
	accountstab.ui
	accountpropsdialog.ui
	operationstab.ui
	operationpropsdialog.ui
	graphstab.ui
	)
QtWrapUi (UIS_H ${FORMS})
CreateTrs ("poleemery" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poleemery" "en;ru_RU" "${SRCS}" "${FORMS}" "poleemerysettings.xml")

add_library (leechcraft_poleemery SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_poleemery
	${LEECHCRAFT_LIBRARIES}
	${QWT_LIBRARIES}
	)
install (TARGETS leechcraft_poleemery DESTINATION ${LC_PLUGINS_DEST})
install (FILES poleemerysettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_poleemery Network Sql Xml)
