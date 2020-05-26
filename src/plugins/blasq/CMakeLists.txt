cmake_minimum_required (VERSION 3.3)
project (leechcraft_blasq)
include (InitLCPlugin NO_POLICY_SCOPE)

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

QtWrapUi (UIS_H ${FORMS})

add_library (leechcraft_blasq SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_blasq
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_blasq DESTINATION ${LC_PLUGINS_DEST})
install (FILES blasqsettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/qml5/blasq DESTINATION ${LC_QML5_DEST})

FindQtLibs (leechcraft_blasq Network QuickWidgets Widgets)

SUBPLUGIN (RAPPOR "Build VKontakte client module for Blasq")
SUBPLUGIN (SPEGNERSI "Build Flickr client module for Blasq")
SUBPLUGIN (VANGOG "Build Picasa client module for Blasq")
SUBPLUGIN (DEATHNOTE "Build LiveJournal FotoBilder client module for Blasq")
