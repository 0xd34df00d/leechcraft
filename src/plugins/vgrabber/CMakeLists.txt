if (NOT QT_USE_FILE)
	cmake_minimum_required (VERSION 2.6)
	if (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	endif ()

	project (leechcraft_vgrabber)

	if (NOT CMAKE_MODULE_PATH)
		set (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	endif ()

	find_package (Boost REQUIRED)
	find_package (Qt4 REQUIRED)
	find_package (LeechCraft REQUIRED)
endif ()

include (${QT_USE_FILE})
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
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_vgrabber SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_vgrabber
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_vgrabber DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES vgrabbersettings.xml DESTINATION ${LC_SETTINGS_DEST})
