cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_birthdaynotifier)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (BIRTHDAYNOTIFIER_SRCS
	birthdaynotifier.cpp
	xmlsettingsmanager.cpp
	)
set (BIRTHDAYNOTIFIER_RESOURCES azothbirthdaynotifierresources.qrc)
QtAddResources (BIRTHDAYNOTIFIER_RCCS ${BIRTHDAYNOTIFIER_RESOURCES})
CreateTrs ("azoth_birthdaynotifier" "en;ro;ru_RU" BIRTHDAYNOTIFIER_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_birthdaynotifier" "en;ru_RU" "${BIRTHDAYNOTIFIER_SRCS}" "" "azothbirthdaynotifiersettings.xml")

add_library (leechcraft_azoth_birthdaynotifier SHARED
	${BIRTHDAYNOTIFIER_COMPILED_TRANSLATIONS}
	${BIRTHDAYNOTIFIER_SRCS}
	${BIRTHDAYNOTIFIER_MOC_SRCS}
	${BIRTHDAYNOTIFIER_RCCS}
	${BIRTHDAYNOTIFIER_RCCS}
	)
target_link_libraries (leechcraft_azoth_birthdaynotifier
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_birthdaynotifier DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothbirthdaynotifiersettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_birthdaynotifier Core Widgets)
