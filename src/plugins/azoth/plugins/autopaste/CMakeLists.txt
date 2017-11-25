cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_autopaste)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

set (AUTOPASTE_SRCS
	actionsstorage.cpp
	autopaste.cpp
	highlight2str.cpp
	bpasteservice.cpp
	codepadservice.cpp
	hastebinservice.cpp
	pasteorgruservice.cpp
	poundpythonservice.cpp
	spacepasteservicebase.cpp
	pastedialog.cpp
	pasteservicebase.cpp
	pasteservicefactory.cpp
	xmlsettingsmanager.cpp
	)

set (AUTOPASTE_FORMS
	pastedialog.ui
	)
set (AUTOPASTE_RESOURCES azothautopasteresources.qrc)

CreateTrs ("azoth_autopaste" "en;es;ru_RU;uk_UA" AUTOPASTE_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_autopaste" "en;ru_RU" "${AUTOPASTE_SRCS}" "${AUTOPASTE_FORMS}" "azothautopastesettings.xml")

QtWrapUi (AUTOPASTE_UIS_H ${AUTOPASTE_FORMS})
QtAddResources (AUTOPASTE_RCCS ${AUTOPASTE_RESOURCES})

add_library (leechcraft_azoth_autopaste SHARED
	${AUTOPASTE_COMPILED_TRANSLATIONS}
	${AUTOPASTE_SRCS}
	${AUTOPASTE_RCCS}
	${AUTOPASTE_UIS_H}
	)
target_link_libraries (leechcraft_azoth_autopaste
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_autopaste DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothautopastesettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_autopaste Network Widgets)
