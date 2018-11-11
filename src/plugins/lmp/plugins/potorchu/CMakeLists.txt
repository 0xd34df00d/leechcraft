cmake_minimum_required (VERSION 3.3)
project (leechcraft_lmp_potorchu)
include (InitLCPlugin NO_POLICY_SCOPE)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (ProjectM)

add_definitions (-DFETCHED_PROJECTM_VERSION="${ProjectM_VERSION}")

include_directories (${LMP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${ProjectM_INCLUDE_DIR}
	)

set (POTORCHU_SRCS
	potorchu.cpp
	visualfilter.cpp
	viswidget.cpp
	visscene.cpp
	)
set (POTORCHU_RESOURCES lmppotorchuresources.qrc)

CreateTrs ("lmp_potorchu" "en;ru_RU" POTORCHU_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("lmp_potorchu" "en;ru_RU" "${POTORCHU_SRCS}" "${POTORCHU_FORMS}" "")
QtAddResources (POTORCHU_RCCS ${POTORCHU_RESOURCES})

add_library (leechcraft_lmp_potorchu SHARED
	${POTORCHU_COMPILED_TRANSLATIONS}
	${POTORCHU_SRCS}
	${POTORCHU_RCCS}
	)
target_link_libraries (leechcraft_lmp_potorchu
	${LEECHCRAFT_LIBRARIES}
	leechcraft_lmp_common
	${GSTREAMER_LIBRARIES}
	${GSTREAMER_INTERFACE_LIBRARY}
	${GLIB2_LIBRARIES}
	${LIBXML2_LIBRARIES}
	${ProjectM_LIBRARIES}
	)

install (TARGETS leechcraft_lmp_potorchu DESTINATION ${LC_PLUGINS_DEST})
#install (FILES lmppotorchufiltersettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_lmp_potorchu OpenGL)
