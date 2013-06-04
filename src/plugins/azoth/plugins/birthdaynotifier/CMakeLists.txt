cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_birthdaynotifier)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (BIRTHDAYNOTIFIER_SRCS
	birthdaynotifier.cpp
	xmlsettingsmanager.cpp
	)
set (BIRTHDAYNOTIFIER_HEADERS
	birthdaynotifier.h
	xmlsettingsmanager.h
	)
set (BIRTHDAYNOTIFIER_RESOURCES azothbirthdaynotifierresources.qrc)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (BIRTHDAYNOTIFIER_MOC_SRCS ${BIRTHDAYNOTIFIER_HEADERS})
endif ()
QT4_ADD_RESOURCES (BIRTHDAYNOTIFIER_RCCS ${BIRTHDAYNOTIFIER_RESOURCES})
CreateTrs ("azoth_birthdaynotifier" "en;ru_RU" BIRTHDAYNOTIFIER_COMPILED_TRANSLATIONS)

add_library (leechcraft_azoth_birthdaynotifier SHARED
	${BIRTHDAYNOTIFIER_COMPILED_TRANSLATIONS}
	${BIRTHDAYNOTIFIER_SRCS}
	${BIRTHDAYNOTIFIER_MOC_SRCS}
	${BIRTHDAYNOTIFIER_RCCS}
	${BIRTHDAYNOTIFIER_RCCS}
	)
target_link_libraries (leechcraft_azoth_birthdaynotifier
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_birthdaynotifier DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothbirthdaynotifiersettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (FILES ${BIRTHDAYNOTIFIER_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
