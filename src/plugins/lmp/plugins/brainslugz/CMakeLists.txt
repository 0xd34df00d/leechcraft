cmake_minimum_required (VERSION 2.8)
project (leechcraft_lmp_brainslugz)
include (InitLCPlugin OPTIONAL)

include_directories (${LMP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (BRAINSLUGZ_SRCS
	brainslugz.cpp
	checktab.cpp
	checkmodel.cpp
	checker.cpp
	progressmodelmanager.cpp
	)

set (BRAINSLUGZ_FORMS
	checktab.ui
	)
QtWrapUi (BRAINSLUGZ_UIS_H ${BRAINSLUGZ_FORMS})
CreateTrs ("lmp_brainslugz" "en;ru_RU" BRAINSLUGZ_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("lmp_brainslugz" "en;ru_RU" "${BRAINSLUGZ_SRCS}" "${BRAINSLUGZ_FORMS}" "")

add_library (leechcraft_lmp_brainslugz SHARED
	${BRAINSLUGZ_COMPILED_TRANSLATIONS}
	${BRAINSLUGZ_SRCS}
	${BRAINSLUGZ_UIS_H}
	)
target_link_libraries (leechcraft_lmp_brainslugz
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_lmp_brainslugz DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${BRAINSLUGZ_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (DIRECTORY share/qml5/lmp DESTINATION ${LC_QML5_DEST})

FindQtLibs (leechcraft_lmp_brainslugz Concurrent QuickWidgets)
