cmake_minimum_required (VERSION 3.3)
project (leechcraft_networkmonitor)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
)
set (SRCS
	networkmonitor.cpp
	requestmodel.cpp
	headermodel.cpp
	)
set (FORMS
	networkmonitor.ui
	)
set (RESOURCES networkmonitorresources.qrc)
CreateTrs ("networkmonitor" "ar;de;en;eo;es;fr;it;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("networkmonitor" "en;ru_RU" "${SRCS}" "${FORMS}" "")

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})
add_library (leechcraft_networkmonitor SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_networkmonitor
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_networkmonitor DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_networkmonitor Network Widgets)
