cmake_minimum_required (VERSION 3.3)
project (leechcraft_lmp_fradj)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (Qwt REQUIRED)

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
	PkgConfig::GStreamer
	${QWT_LIBRARIES}
	)

install (TARGETS leechcraft_lmp_fradj DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_lmp_fradj Widgets)
