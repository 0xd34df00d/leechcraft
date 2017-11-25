cmake_minimum_required (VERSION 3.3)
project (leechcraft_dolozhee)
include (InitLCPlugin)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	dolozhee.cpp
	xmlgenerator.cpp
	reportwizard.cpp
	chooseuserpage.cpp
	userstatuspage.cpp
	reporttypepage.cpp
	bugreportpage.cpp
	featurerequestpage.cpp
	fileattachpage.cpp
	finalpage.cpp
	previewpage.cpp
	)
set (FORMS
	chooseuserpage.ui
	userstatuspage.ui
	reporttypepage.ui
	bugreportpage.ui
	featurerequestpage.ui
	fileattachpage.ui
	finalpage.ui
	previewpage.ui
	)
CreateTrs ("dolozhee" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("dolozhee" "en;ru_RU" "${SRCS}" "${FORMS}" "")
QtWrapUi (UIS_H ${FORMS})

add_library (leechcraft_dolozhee SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_dolozhee
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_dolozhee DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_dolozhee Network Widgets Xml)
