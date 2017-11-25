cmake_minimum_required (VERSION 3.3)
project (leechcraft_auscrie)
include (InitLCPlugin)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	auscrie.cpp
	shooterdialog.cpp
	util.cpp
	)
set (FORMS
	shooterdialog.ui
	)
set (RESOURCES auscrieresources.qrc)
CreateTrs ("auscrie" "de;en;es;fr;it;oc;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("auscrie" "en;ru_RU" "${SRCS}" "${FORMS}" "")
QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_auscrie SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_auscrie
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_auscrie DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_auscrie Widgets)
