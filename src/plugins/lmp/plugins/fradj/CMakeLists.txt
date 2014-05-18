cmake_minimum_required (VERSION 2.8)
project (leechcraft_lmp_fradj)
include (InitLCPlugin OPTIONAL)

include_directories (${LMP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (FRADJ_SRCS
	fradj.cpp
	)

#CreateTrs ("lmp_fradj" "en;ru_RU" FRADJ_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("lmp_fradj" "en;ru_RU" "${FRADJ_SRCS}" "${FRADJ_FORMS}" "lmpfradjfiltersettings.xml")

add_library (leechcraft_lmp_fradj SHARED
	${FRADJ_COMPILED_TRANSLATIONS}
	${FRADJ_SRCS}
	)
target_link_libraries (leechcraft_lmp_fradj
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	leechcraft_lmp_common
	${GSTREAMER_LIBRARIES}
	${GLIB2_LIBRARIES}
	${LIBXML2_LIBRARIES}
	)

install (TARGETS leechcraft_lmp_fradj DESTINATION ${LC_PLUGINS_DEST})
install (FILES lmpfradjfiltersettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (FILES ${FRADJ_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
