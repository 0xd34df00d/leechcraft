cmake_minimum_required (VERSION 2.8)
project (leechcraft_blasq_vangog)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (VANGOG_SRCS
	vangog.cpp
	picasaservice.cpp
	picasaaccount.cpp
	)
# CreateTrs("blasq_vangog" "en;ru_RU" VANGOG_COMPILED_TRANSLATIONS)
# CreateTrsUpTarget("blasq_vangog" "en;ru_RU" "${VANGOG_SRCS}" "${VANGOG_FORMS}" "")

add_library (leechcraft_blasq_vangog SHARED
# 	${VANGOG_COMPILED_TRANSLATIONS}
	${VANGOG_SRCS}
	)
target_link_libraries (leechcraft_blasq_vangog
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_blasq_vangog DESTINATION ${LC_PLUGINS_DEST})

