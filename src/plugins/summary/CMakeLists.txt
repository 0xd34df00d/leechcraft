cmake_minimum_required (VERSION 2.8)
project (leechcraft_summary)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	summary.cpp
	core.cpp
	summarywidget.cpp
	summarytagsfilter.cpp
	modeldelegate.cpp
	)
set (FORMS
	summarywidget.ui
	)
set (RESOURCES summaryresources.qrc)

CreateTrs ("summary" "ar;be;de;en;es;fr;it;ro;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("summary" "en;ru_RU" "${SRCS}" "${FORMS}" "")

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_summary SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_summary
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_summary DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_summary Widgets)
