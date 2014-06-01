cmake_minimum_required (VERSION 2.8)
project (leechcraft_monocle_pdf)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (PopplerQt4 REQUIRED)

include (${QT_USE_FILE})
include_directories (
	${POPPLER_QT4_INCLUDE_DIR}
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
	)

set (PDF_RESOURCES pdfresources.qrc)
QT4_ADD_RESOURCES (PDF_RCCS ${PDF_RESOURCES})

add_library (leechcraft_monocle_pdf SHARED
	${SRCS}
	${MOC_SRCS}
	${PDF_RCCS}
	)
target_link_libraries (leechcraft_monocle_pdf
	${POPPLER_QT4_LIBRARIES}
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_monocle_pdf DESTINATION ${LC_PLUGINS_DEST})
if (UNIX AND NOT APPLE)
	install (FILES freedesktop/leechcraft-monocle-pdf.desktop DESTINATION share/applications)
endif ()
