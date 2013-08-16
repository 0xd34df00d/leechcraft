cmake_minimum_required (VERSION 2.8)
project (leechcraft_blasq_rappor)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (RAPPOR_SRCS
	rappor.cpp
	vkservice.cpp
	vkaccount.cpp
	albumsettingsdialog.cpp
	)
set (RAPPOR_FORMS
	albumsettingsdialog.ui
	)
CreateTrs("blasq_rappor" "en;ru_RU" RAPPOR_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("blasq_rappor" "en;ru_RU" "${RAPPOR_SRCS}" "${RAPPOR_FORMS}" "")

QT4_WRAP_UI (RAPPOR_UIS_H ${RAPPOR_FORMS})

add_library (leechcraft_blasq_rappor SHARED
	${RAPPOR_COMPILED_TRANSLATIONS}
	${RAPPOR_SRCS}
	${RAPPOR_UIS_H}
	)
target_link_libraries (leechcraft_blasq_rappor
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QTKOAUTH_LIBRARIES}
	)
install (TARGETS leechcraft_blasq_rappor DESTINATION ${LC_PLUGINS_DEST})

