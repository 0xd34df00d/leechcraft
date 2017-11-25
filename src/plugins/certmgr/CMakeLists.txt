cmake_minimum_required (VERSION 3.3)
project (leechcraft_certmgr)
include (InitLCPlugin)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	certmgr.cpp
	xmlsettingsmanager.cpp
	acceptedrejecteddialog.cpp
	manager.cpp
	certsmodel.cpp
	managerdialog.cpp
	exceptionsmodel.cpp
	)
set (FORMS
	acceptedrejecteddialog.ui
	managerdialog.ui
	)

QtWrapUi (UIS_H ${FORMS})

CreateTrs("certmgr" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("certmgr" "en;ru_RU" "${SRCS}" "${FORMS}" "certmgrsettings.xml")

add_library (leechcraft_certmgr SHARED
	${COMPILED_TRANSLATIONS}
	${UIS_H}
	${SRCS}
	)
target_link_libraries (leechcraft_certmgr
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_certmgr DESTINATION ${LC_PLUGINS_DEST})
install (FILES certmgrsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_certmgr Network Widgets)
