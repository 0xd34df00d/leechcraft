cmake_minimum_required (VERSION 3.3)
project (leechcraft_choroid)
include (InitLCPlugin)

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
#CreateTrs("blasq" "en;ru_RU" COMPILED_TRANSLATIONS)
#CreateTrsUpTarget("blasq" "en;ru_RU" "${SRCS}" "${FORMS}" "")

QtWrapUi (UIS_H ${FORMS})

add_library (leechcraft_choroid SHARED
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_choroid
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_choroid DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (DIRECTORY share/qml5/choroid DESTINATION ${LC_QML5_DEST})

FindQtLibs (leechcraft_choroid QuickWidgets Widgets)
