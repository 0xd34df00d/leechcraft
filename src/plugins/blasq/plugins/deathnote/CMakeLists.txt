cmake_minimum_required (VERSION 3.3)
project (leechcraft_blasq_deathnote)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (DEATHNOTE_SRCS
	albumsettingsdialog.cpp
	deathnote.cpp
	fotobilderaccount.cpp
	fotobilderservice.cpp
	registerpage.cpp
	selectgroupsdialog.cpp
	util.cpp
	)

set (DEATHNOTE_FORMS
	albumsettingsdialog.ui
	registerpage.ui
	selectgroupsdialog.ui
	)

set (DEATHNOTE_RESOURCES blasqdeathnoteresources.qrc)
CreateTrs("blasq_deathnote" "en;ru_RU" DEATHNOTE_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("blasq_deathnote" "en;ru_RU" "${DEATHNOTE_SRCS}" "${DEATHNOTE_FORMS}" "")

QtWrapUi (DEATHNOTE_UIS_H ${DEATHNOTE_FORMS})
QtAddResources (DEATHNOTE_RCCS ${DEATHNOTE_RESOURCES})

add_library (leechcraft_blasq_deathnote SHARED
	${DEATHNOTE_COMPILED_TRANSLATIONS}
	${DEATHNOTE_SRCS}
	${DEATHNOTE_UIS_H}
	${DEATHNOTE_RCCS}
	)

target_link_libraries (leechcraft_blasq_deathnote
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_blasq_deathnote DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_blasq_deathnote Gui Network Widgets Xml XmlPatterns)
