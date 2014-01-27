cmake_minimum_required (VERSION 2.8)
project (leechcraft_blasq)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTDECLARATIVE TRUE)
set (QT_USE_QTNETWORK TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	blasq.cpp
	xmlsettingsmanager.cpp
	accountswidget.cpp
	newaccountwizardfirstpage.cpp
	servicesmanager.cpp
	accountsmanager.cpp
	photostab.cpp
	defaultimagechooser.cpp
	enumsproxy.cpp
	uploadphotosdialog.cpp
	selectalbumdialog.cpp
	photosproxymodel.cpp
	datafilteruploader.cpp
	)
set (FORMS
	accountswidget.ui
	newaccountwizardfirstpage.ui
	photostab.ui
	uploadphotosdialog.ui
	selectalbumdialog.ui
	)
CreateTrs("blasq" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("blasq" "en;ru_RU" "${SRCS}" "${FORMS}" "")

QT4_WRAP_UI (UIS_H ${FORMS})

add_library (leechcraft_blasq SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_blasq
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_blasq DESTINATION ${LC_PLUGINS_DEST})
install (FILES blasqsettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/qml/blasq DESTINATION ${LC_QML_DEST})

option (ENABLE_BLASQ_RAPPOR "Build VKontakte client module for Blasq" ON)
option (ENABLE_BLASQ_SPEGNERSI "Build Flickr client module for Blasq" ON)
option (ENABLE_BLASQ_VANGOG "Build Picasa client module for Blasq" ON)
option (ENABLE_BLASQ_DEATHNOTE "Build LiveJournal FotoBilder client module for Blasq" ON)

if (ENABLE_BLASQ_RAPPOR)
	add_subdirectory (plugins/rappor)
endif ()

if (ENABLE_BLASQ_SPEGNERSI)
	add_subdirectory (plugins/spegnersi)
endif ()

if (ENABLE_BLASQ_VANGOG)
	add_subdirectory (plugins/vangog)
endif ()

if (ENABLE_BLASQ_DEATHNOTE)
	add_subdirectory (plugins/deathnote)
endif ()
