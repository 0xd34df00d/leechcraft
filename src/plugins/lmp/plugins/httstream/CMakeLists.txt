cmake_minimum_required (VERSION 3.3)
project (leechcraft_lmp_httstream)
include (InitLCPlugin)

include_directories (${LMP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (HTTSTREAM_SRCS
	httstream.cpp
	httpserver.cpp
	httpstreamfilter.cpp
	filterconfigurator.cpp
	)

CreateTrs ("lmp_httstream" "en;ru_RU" HTTSTREAM_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("lmp_httstream" "en;ru_RU" "${HTTSTREAM_SRCS}" "${HTTSTREAM_FORMS}" "lmphttstreamfiltersettings.xml")

add_library (leechcraft_lmp_httstream SHARED
	${HTTSTREAM_COMPILED_TRANSLATIONS}
	${HTTSTREAM_SRCS}
	)
target_link_libraries (leechcraft_lmp_httstream
	${LEECHCRAFT_LIBRARIES}
	leechcraft_lmp_common
	${GSTREAMER_LIBRARIES}
	${GLIB2_LIBRARIES}
	${LIBXML2_LIBRARIES}
	)

install (TARGETS leechcraft_lmp_httstream DESTINATION ${LC_PLUGINS_DEST})
install (FILES lmphttstreamfiltersettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (FILES ${HTTSTREAM_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})

FindQtLibs (leechcraft_lmp_httstream Gui Network)
