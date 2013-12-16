cmake_minimum_required (VERSION 2.8)
project (leechcraft_blasq_deathnote)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
set (QT_USE_QTXMLPATTERNS TRUE)
include (${QT_USE_FILE})

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (DEATHNOTE_SRCS
	albumsettingsdialog.cpp
	deathnote.cpp
	fotobilderaccount.cpp
	fotobilderservice.cpp
	registerpage.cpp
	selectgroupsdialog.cpp
	)

set (DEATHNOTE_FORMS
	albumsettingsdialog.ui
	registerpage.ui
	selectgroupsdialog.ui
	)

set (DEATHNOTE_RESOURCES blasqdeathnoteresources.qrc)
CreateTrs("blasq_deathnote" "en;ru_RU" DEATHNOTE_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("blasq_deathnote" "en;ru_RU" "${DEATHNOTE_SRCS}" "${DEATHNOTE_FORMS}" "")

QT4_WRAP_UI (DEATHNOTE_UIS_H ${DEATHNOTE_FORMS})
QT4_ADD_RESOURCES (DEATHNOTE_RCCS ${DEATHNOTE_RESOURCES})

add_library (leechcraft_blasq_deathnote SHARED
	${DEATHNOTE_COMPILED_TRANSLATIONS}
	${DEATHNOTE_SRCS}
	${DEATHNOTE_UIS_H}
	${DEATHNOTE_RCCS}
	)

target_link_libraries (leechcraft_blasq_deathnote
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_blasq_deathnote DESTINATION ${LC_PLUGINS_DEST})
