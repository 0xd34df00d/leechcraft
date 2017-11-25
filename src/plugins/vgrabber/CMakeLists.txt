cmake_minimum_required (VERSION 3.3)
project (leechcraft_vgrabber)
include (InitLCPlugin)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	vgrabber.cpp
	findproxy.cpp
	categoriesselector.cpp
	categorymodifier.cpp
	xmlsettingsmanager.cpp
	audiofindproxy.cpp
	videofindproxy.cpp
	)
set (FORMS
	categoriesselector.ui
	categorymodifier.ui
	)
set (RESOURCES vgrabberresources.qrc)
CreateTrs ("vgrabber" "ar;de;en;fr;it;oc;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_vgrabber SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_vgrabber
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_vgrabber DESTINATION ${LC_PLUGINS_DEST})
install (FILES vgrabbersettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_vgrabber Widgets)
