cmake_minimum_required (VERSION 3.3)
project (leechcraft_blasq_rappor)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (RAPPOR_SRCS
	rappor.cpp
	vkservice.cpp
	vkaccount.cpp
	albumsettingsdialog.cpp
	uploadmanager.cpp
	)
set (RAPPOR_FORMS
	albumsettingsdialog.ui
	)
set (RAPPOR_RESOURCES
	blasqrapporresources.qrc
	)
CreateTrs("blasq_rappor" "en;ru_RU" RAPPOR_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("blasq_rappor" "en;ru_RU" "${RAPPOR_SRCS}" "${RAPPOR_FORMS}" "")

QtWrapUi (RAPPOR_UIS_H ${RAPPOR_FORMS})
QtAddResources (RAPPOR_RCCS ${RAPPOR_RESOURCES})

add_library (leechcraft_blasq_rappor SHARED
	${RAPPOR_COMPILED_TRANSLATIONS}
	${RAPPOR_SRCS}
	${RAPPOR_UIS_H}
	${RAPPOR_RCCS}
	)
target_link_libraries (leechcraft_blasq_rappor
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_blasq_rappor DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_blasq_rappor Gui Network Widgets Xml)
