cmake_minimum_required (VERSION 3.3)
project (leechcraft_cstp)
include (InitLCPlugin OPTIONAL)

include_directories (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
	cstp.cpp
	core.cpp
	task.cpp
	addtask.cpp
	xmlsettingsmanager.cpp
	)
set (FORMS
	addtask.ui
	)
set (RESOURCES
	cstpresources.qrc
	)
CreateTrs ("cstp" "ar;de;en;eo;es;fr;it;oc;pl;ro;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("cstp" "en;ru_RU" "${SRCS}" "${FORMS}" "cstpsettings.xml")
QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_cstp SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS})
link_directories (${Boost_LIBRARY_DIR})
target_link_libraries (leechcraft_cstp
						${LEECHCRAFT_LIBRARIES}
						)
install (TARGETS leechcraft_cstp DESTINATION ${LC_PLUGINS_DEST})
install (FILES cstpsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_cstp Gui Network Widgets)
