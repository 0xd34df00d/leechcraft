cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_xtazy)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (XTAZY_SRCS
	xtazy.cpp
	xmlsettingsmanager.cpp
	tracksharedialog.cpp
	)
set (XTAZY_FORMS
	tracksharedialog.ui
	)
set (XTAZY_RESOURCES azothxtazyresources.qrc)

CreateTrs ("azoth_xtazy" "en;es;ru_RU;uk_UA" XTAZY_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_xtazy" "en;ru_RU" "${XTAZY_SRCS}" "${XTAZY_FORMS}" "azothxtazysettings.xml")

QtAddResources (XTAZY_RCCS ${XTAZY_RESOURCES})
QtWrapUi (XTAZY_UIS_H ${XTAZY_FORMS})

add_library (leechcraft_azoth_xtazy SHARED
	${XTAZY_COMPILED_TRANSLATIONS}
	${XTAZY_SRCS}
	${XTAZY_RCCS}
	${XTAZY_UIS_H}
	)
target_link_libraries (leechcraft_azoth_xtazy
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_xtazy DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothxtazysettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_xtazy Widgets)
