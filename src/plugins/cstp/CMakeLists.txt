cmake_minimum_required (VERSION 2.8)
project (leechcraft_cstp)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTNETWORK TRUE)
include (${QT_USE_FILE})
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
CreateTrs ("cstp" "ar;de;en;eo;es;fr;it;oc;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_cstp SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS})
link_directories (${Boost_LIBRARY_DIR})
target_link_libraries (leechcraft_cstp
                        ${QT_LIBRARIES}
						${LEECHCRAFT_LIBRARIES}
						)
install (TARGETS leechcraft_cstp RUNTIME DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES cstpsettings.xml DESTINATION ${LC_SETTINGS_DEST})
