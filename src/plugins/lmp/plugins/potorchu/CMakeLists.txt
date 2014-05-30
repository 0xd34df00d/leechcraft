cmake_minimum_required (VERSION 2.8)
project (leechcraft_lmp_potorchu)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTOPENGL TRUE)
include (${QT_USE_FILE})

include_directories (${LMP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (POTORCHU_SRCS
	potorchu.cpp
	visualfilter.cpp
	viswidget.cpp
	visscene.cpp
	)

#CreateTrs ("lmp_potorchu" "en;ru_RU" POTORCHU_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("lmp_potorchu" "en;ru_RU" "${POTORCHU_SRCS}" "${POTORCHU_FORMS}" "")

add_library (leechcraft_lmp_potorchu SHARED
	${POTORCHU_COMPILED_TRANSLATIONS}
	${POTORCHU_SRCS}
	)
target_link_libraries (leechcraft_lmp_potorchu
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	leechcraft_lmp_common
	${GSTREAMER_LIBRARIES}
	${GSTREAMER_INTERFACE_LIBRARY}
	${GLIB2_LIBRARIES}
	${LIBXML2_LIBRARIES}
	projectM
	)

install (TARGETS leechcraft_lmp_potorchu DESTINATION ${LC_PLUGINS_DEST})
#install (FILES lmppotorchufiltersettings.xml DESTINATION ${LC_SETTINGS_DEST})
#install (FILES ${POTORCHU_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
