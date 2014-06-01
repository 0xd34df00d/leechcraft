if (NOT QT_USE_FILE)
	cmake_minimum_required (VERSION 2.6)
	if (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	endif ()

	project (leechcraft_newlife)

	if (NOT CMAKE_MODULE_PATH)
		set (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	endif ()

	find_package (Boost REQUIRED)
	find_package (Qt4 REQUIRED)
	find_package (LeechCraft REQUIRED)
endif ()

set (QT_USE_QTXML TRUE)
set (QT_USE_QTSQL TRUE)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
	newlife.cpp
	importwizard.cpp
	firstpage.cpp
	abstractimporter.cpp
	imhistimporterbase.cpp
	common/imimporter.cpp
	common/imimportpage.cpp
	common/xmlimaccount.cpp
	importers/akregator/akregatorimporter.cpp
	importers/akregator/akregatorimportpage.cpp
	importers/liferea/lifereaimportpage.cpp
	importers/liferea/lifereaimporter.cpp
	importers/kopete/kopeteimporter.cpp
	importers/kopete/kopeteimportpage.cpp
	importers/kopete/kopeteimportthread.cpp
	importers/ktorrent/ktorrentimportpage.cpp
	importers/ktorrent/ktorrentimporter.cpp
	importers/firefox/firefoximportpage.cpp
	importers/firefox/firefoximporter.cpp
	importers/firefox/firefoxprofileselectpage.cpp
	importers/psiplus/psiplusimporter.cpp
	importers/psiplus/psiplusimportpage.cpp
	importers/vacuum/vacuumimporter.cpp
	importers/vacuum/vacuumimportpage.cpp
	importers/opera/operaimporter.cpp
	importers/opera/operaimportselectpage.cpp
	)
set (FORMS
	importwizard.ui
	feedssettingsimportpage.ui
	firstpage.ui
	common/imimportpage.ui
	importers/firefox/firefoxprofileselectpage.ui
	importers/opera/operaimportselectpage.ui
	)
set (RESOURCES newliferesources.qrc)
CreateTrs ("newlife" "ar;be;de;en;fr;it;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("newlife" "en;ru_RU" "${SRCS}" "${FORMS}" "")

QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_newlife SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_newlife
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_newlife DESTINATION ${LC_PLUGINS_DEST})
