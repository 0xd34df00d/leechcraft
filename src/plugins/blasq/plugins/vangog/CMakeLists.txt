cmake_minimum_required (VERSION 2.8)
project (leechcraft_blasq_vangog)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})

find_package (QJSON REQUIRED)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QJSON_INCLUDE_DIR}
	)

set (VANGOG_SRCS
	authmanager.cpp
	picasaaccount.cpp
	picasamanager.cpp
	picasaservice.cpp
	registerpage.cpp
	vangog.cpp
	)

set (VANGOG_FORMS
	registerpage.ui
	)
# CreateTrs("blasq_vangog" "en;ru_RU" VANGOG_COMPILED_TRANSLATIONS)
# CreateTrsUpTarget("blasq_vangog" "en;ru_RU" "${VANGOG_SRCS}" "${VANGOG_FORMS}" "")
QT4_WRAP_UI (VANGOG_UIS_H ${VANGOG_FORMS})

add_library (leechcraft_blasq_vangog SHARED
# 	${VANGOG_COMPILED_TRANSLATIONS}
	${VANGOG_SRCS}
    ${VANGOG_UIS_H}
	)
target_link_libraries (leechcraft_blasq_vangog
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QJSON_LIBRARIES}
	)
install (TARGETS leechcraft_blasq_vangog DESTINATION ${LC_PLUGINS_DEST})

