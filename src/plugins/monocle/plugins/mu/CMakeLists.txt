cmake_minimum_required (VERSION 2.8)
project (leechcraft_monocle_mu)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (MUPDF_VERSION)
	add_definitions (-DMUPDF_VERSION=${MUPDF_VERSION})
else (MUPDF_VERSION)
	message (WARNING "MuPDF version is not set, defaulting to 1.1")
	add_definitions (-DMUPDF_VERSION=0x0101)
endif (MUPDF_VERSION)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	mu.cpp
	document.cpp
	)

add_library (leechcraft_monocle_mu SHARED
	${SRCS}
	)
target_link_libraries (leechcraft_monocle_mu
	fitz
	jpeg
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_monocle_mu DESTINATION ${LC_PLUGINS_DEST})
