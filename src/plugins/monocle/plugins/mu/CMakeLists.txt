cmake_minimum_required (VERSION 3.3)
project (leechcraft_monocle_mu)
include (InitLCPlugin NO_POLICY_SCOPE)

if (MUPDF_VERSION)
	add_definitions (-DMUPDF_VERSION=${MUPDF_VERSION})
else ()
	message (WARNING "MuPDF version is not set, defaulting to 1.2")
	add_definitions (-DMUPDF_VERSION=0x0102)
endif ()

LC_DEFINE_PLUGIN (
	SRCS mu.cpp document.cpp
	QT_COMPONENTS Core
	LINK_LIBRARIES fitz jbig2dec jpeg openjpeg
	)
