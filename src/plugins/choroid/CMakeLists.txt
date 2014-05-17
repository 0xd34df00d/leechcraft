cmake_minimum_required (VERSION 2.8)
project (leechcraft_choroid)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTDECLARATIVE TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	choroid.cpp
	choroidtab.cpp
	)
set (FORMS
	choroidtab.ui
	)
#set (TRANSLATIONS
#	leechcraft_choroid_ru_RU.ts
#	leechcraft_choroid_en.ts
#	)
#set (COMPILED_TRANSLATIONS
#	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_choroid_ru_RU.qm
#	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_choroid_en.qm
#	)
QT4_WRAP_UI (UIS_H ${FORMS})

#add_custom_command (OUTPUT
#	${COMPILED_TRANSLATIONS}
#	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
#	DEPENDS ${TRANSLATIONS}
#	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#	)
add_library (leechcraft_choroid SHARED
#	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_choroid
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_choroid DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (DIRECTORY share/qml/choroid DESTINATION ${LC_QML_DEST})
