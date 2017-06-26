cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_metacontacts)
include (InitLCPlugin OPTIONAL)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (METACONTACTS_SRCS
	metacontacts.cpp
	metaprotocol.cpp
	metaaccount.cpp
	metaentry.cpp
	metamessage.cpp
	core.cpp
	addtometacontactsdialog.cpp
	managecontactsdialog.cpp
	)
set (METACONTACTS_FORMS
	addtometacontactsdialog.ui
	managecontactsdialog.ui
	)
set (METACONTACTS_RESOURCES azothmetacontactsresources.qrc)
CreateTrs ("azoth_metacontacts" "en;es;ro;ru_RU;uk_UA" METACONTACTS_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_metacontacts" "en;ru_RU" "${METACONTACTS_SRCS}" "${METACONTACTS_FORMS}" "")
QtWrapUi (METACONTACTS_UIS_H ${METACONTACTS_FORMS})
QtAddResources (METACONTACTS_RCCS ${METACONTACTS_RESOURCES})

add_library (leechcraft_azoth_metacontacts SHARED
	${METACONTACTS_COMPILED_TRANSLATIONS}
	${METACONTACTS_SRCS}
	${METACONTACTS_UIS_H}
	${METACONTACTS_RCCS}
	)
target_link_libraries (leechcraft_azoth_metacontacts
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_metacontacts DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_azoth_metacontacts Widgets)
