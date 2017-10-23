cmake_minimum_required (VERSION 3.3)
project (leechcraft_monocle_mu)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (MUPDF_VERSION)
	add_definitions (-DMUPDF_VERSION=${MUPDF_VERSION})
else ()
	message (WARNING "MuPDF version is not set, defaulting to 1.2")
	add_definitions (-DMUPDF_VERSION=0x0102)
endif ()

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
	jbig2dec
	jpeg
	openjpeg
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_monocle_mu DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_monocle_mu Core)
