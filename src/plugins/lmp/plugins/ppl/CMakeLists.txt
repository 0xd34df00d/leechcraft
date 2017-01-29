cmake_minimum_required (VERSION 2.8)
project (leechcraft_lmp_ppl)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories (${LMP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (PPL_SRCS
	ppl.cpp
	)

add_definitions (${MTP_DEFINITIONS})

add_library (leechcraft_lmp_ppl SHARED
	${PPL_COMPILED_TRANSLATIONS}
	${PPL_SRCS}
	)
target_link_libraries (leechcraft_lmp_ppl
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_lmp_ppl DESTINATION ${LC_PLUGINS_DEST})
#install (FILES lmppplsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_lmp_ppl Widgets)
