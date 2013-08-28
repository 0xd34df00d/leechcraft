cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_metacontacts)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
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
set (METACONTACTS_HEADERS
	metacontacts.h
	metaprotocol.h
	metaaccount.h
	metaentry.h
	metamessage.h
	core.h
	addtometacontactsdialog.h
	managecontactsdialog.h
	)
set (METACONTACTS_FORMS
	addtometacontactsdialog.ui
	managecontactsdialog.ui
	)
set (METACONTACTS_RESOURCES azothmetacontactsresources.qrc)
CreateTrs ("azoth_metacontacts" "en;es;ru_RU;uk_UA" METACONTACTS_COMPILED_TRANSLATIONS)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (METACONTACTS_MOC_SRCS ${METACONTACTS_HEADERS})
endif ()
QT4_WRAP_UI (METACONTACTS_UIS_H ${METACONTACTS_FORMS})
QT4_ADD_RESOURCES (METACONTACTS_RCCS ${METACONTACTS_RESOURCES})

add_library (leechcraft_azoth_metacontacts SHARED
	${METACONTACTS_COMPILED_TRANSLATIONS}
	${METACONTACTS_SRCS}
	${METACONTACTS_MOC_SRCS}
	${METACONTACTS_UIS_H}
	${METACONTACTS_RCCS}
	)
target_link_libraries (leechcraft_azoth_metacontacts
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_metacontacts RUNTIME DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${METACONTACTS_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
