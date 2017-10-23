cmake_minimum_required (VERSION 3.3)
project (leechcraft_lmp_fradj)
include (InitLCPlugin OPTIONAL)

set (ADDITIONAL_LIBRARIES)

if (WITH_QWT)
	add_definitions (-DWITH_QWT)
	find_package (Qwt)

	set (ADDITIONAL_LIBRARIES ${QWT_LIBRARIES})
	include_directories (${QWT_INCLUDE_DIRS})
endif (WITH_QWT)

include_directories (${LMP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QWT_INCLUDE_DIRS}
	)

set (FRADJ_SRCS
	fradj.cpp
	eq10bandeffect.cpp
	eqconfigurator.cpp
	eqconfiguratordialog.cpp
	eqbandwidget.cpp
	bandinfo.cpp
	)

set (FRADJ_FORMS
	eqbandwidget.ui
	eqconfiguratordialog.ui
	)
QtWrapUi (FRADJ_UIS_H ${FRADJ_FORMS})
CreateTrs ("lmp_fradj" "en;ru_RU" FRADJ_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("lmp_fradj" "en;ru_RU" "${FRADJ_SRCS}" "${FRADJ_FORMS}" "")

add_library (leechcraft_lmp_fradj SHARED
	${FRADJ_COMPILED_TRANSLATIONS}
	${FRADJ_SRCS}
	${FRADJ_UIS_H}
	)
target_link_libraries (leechcraft_lmp_fradj
	${LEECHCRAFT_LIBRARIES}
	leechcraft_lmp_common
	${GSTREAMER_LIBRARIES}
	${GLIB2_LIBRARIES}
	${LIBXML2_LIBRARIES}
	${ADDITIONAL_LIBS}
	)

install (TARGETS leechcraft_lmp_fradj DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_lmp_fradj Widgets)
