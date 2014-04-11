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
	)

#CreateTrs ("lmp_brainslugz" "en;ru_RU" BRAINSLUGZ_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("lmp_brainslugz" "en;ru_RU" "${BRAINSLUGZ_SRCS}" "${BRAINSLUGZ_FORMS}" "")

add_library (leechcraft_lmp_brainslugz SHARED
	${BRAINSLUGZ_COMPILED_TRANSLATIONS}
	${BRAINSLUGZ_SRCS}
	)
target_link_libraries (leechcraft_lmp_brainslugz
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_lmp_brainslugz DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${BRAINSLUGZ_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
