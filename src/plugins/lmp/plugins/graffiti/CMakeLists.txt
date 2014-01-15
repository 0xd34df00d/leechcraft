cmake_minimum_required (VERSION 2.8)
project (leechcraft_lmp_graffiti)
include (InitLCPlugin OPTIONAL)

find_package (Taglib REQUIRED)
add_definitions (${TAGLIB_CFLAGS})

include (${QT_USE_FILE})
include_directories (${LMP_INCLUDE_DIR}
	${TAGLIB_INCLUDES}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (GRAFFITI_SRCS
	graffiti.cpp
	graffititab.cpp
	filesmodel.cpp
	renamedialog.cpp
	fileswatcher.cpp
	cuesplitter.cpp
	progressmanager.cpp
	tagsfetchmanager.cpp
	reciterator.cpp
	)
set (GRAFFITI_FORMS
	graffititab.ui
	renamedialog.ui
	)
QT4_WRAP_UI (GRAFFITI_UIS_H ${GRAFFITI_FORMS})
CreateTrs ("lmp_graffiti" "en;ru_RU" GRAFFITI_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("lmp_graffiti" "en;ru_RU" "${GRAFFITI_SRCS}" "${GRAFFITI_FORMS}" "")

add_definitions (${MTP_DEFINITIONS})

set (GRAFFITI_RESOURCES graffitiresources.qrc)
QT4_ADD_RESOURCES (GRAFFITI_RCCS ${GRAFFITI_RESOURCES})

add_library (leechcraft_lmp_graffiti SHARED
	${GRAFFITI_RCCS}
	${GRAFFITI_COMPILED_TRANSLATIONS}
	${GRAFFITI_SRCS}
	${GRAFFITI_UIS_H}
	)
target_link_libraries (leechcraft_lmp_graffiti
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${TAGLIB_LIBRARIES}
	leechcraft_lmp_common
	)

install (TARGETS leechcraft_lmp_graffiti DESTINATION ${LC_PLUGINS_DEST})
#install (FILES lmpgraffitisettings.xml DESTINATION ${LC_SETTINGS_DEST})
