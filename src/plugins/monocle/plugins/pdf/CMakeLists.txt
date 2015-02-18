cmake_minimum_required (VERSION 2.8)
project (leechcraft_monocle_pdf)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (NOT USE_QT5)
	find_package (PopplerQt4 REQUIRED)
	set (POPPLER_QT_INCLUDE_DIR ${POPPLER_QT4_INCLUDE_DIR})
	set (POPPLER_QT_LIBRARIES ${POPPLER_QT4_LIBRARIES})
else ()
	find_package (PopplerQt5 REQUIRED)
	set (POPPLER_QT_INCLUDE_DIR ${POPPLER_QT5_INCLUDE_DIR})
	set (POPPLER_QT_LIBRARIES ${POPPLER_QT5_LIBRARIES})
endif ()

include_directories (
	${POPPLER_QT_INCLUDE_DIR}
	${POPPLER_CPP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	pdf.cpp
	document.cpp
	links.cpp
	fields.cpp
	annotations.cpp
	xmlsettingsmanager.cpp
	pendingfontinforequest.cpp
	)

set (PDF_RESOURCES pdfresources.qrc)
CreateTrs ("monocle_pdf" "en;ru_RU" PDF_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("monocle_pdf" "en;ru_RU" "${SRCS}" "" "monoclepdfsettings.xml")
QtAddResources (PDF_RCCS ${PDF_RESOURCES})

add_library (leechcraft_monocle_pdf SHARED
	${PDF_COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${PDF_RCCS}
	)
target_link_libraries (leechcraft_monocle_pdf
	${POPPLER_QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_monocle_pdf DESTINATION ${LC_PLUGINS_DEST})
install (FILES monoclepdfsettings.xml DESTINATION ${LC_SETTINGS_DEST})
if (UNIX AND NOT APPLE)
	if (USE_QT5)
		install (FILES freedesktop/leechcraft-monocle-pdf-qt5.desktop DESTINATION share/applications)
	else ()
		install (FILES freedesktop/leechcraft-monocle-pdf.desktop DESTINATION share/applications)
	endif ()
endif ()

FindQtLibs (leechcraft_monocle_pdf Concurrent)
