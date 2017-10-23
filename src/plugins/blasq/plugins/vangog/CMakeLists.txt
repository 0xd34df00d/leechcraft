cmake_minimum_required (VERSION 3.3)
project (leechcraft_blasq_vangog)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (VANGOG_SRCS
	albumsettingsdialog.cpp
	authmanager.cpp
	picasaaccount.cpp
	picasamanager.cpp
	picasaservice.cpp
	registerpage.cpp
	uploadmanager.cpp
	vangog.cpp
	)

set (VANGOG_FORMS
	albumsettingsdialog.ui
	registerpage.ui
	)

set (VANGOG_RESOURCES blasqvangogresources.qrc)
CreateTrs("blasq_vangog" "en;ru_RU" VANGOG_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("blasq_vangog" "en;ru_RU" "${VANGOG_SRCS}" "${VANGOG_FORMS}" "")

QtWrapUi (VANGOG_UIS_H ${VANGOG_FORMS})
QtAddResources (VANGOG_RCCS ${VANGOG_RESOURCES})

add_library (leechcraft_blasq_vangog SHARED
	${VANGOG_COMPILED_TRANSLATIONS}
	${VANGOG_SRCS}
	${VANGOG_UIS_H}
	${VANGOG_RCCS}
	)
target_link_libraries (leechcraft_blasq_vangog
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_blasq_vangog DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_blasq_vangog Gui Network Widgets Xml)
