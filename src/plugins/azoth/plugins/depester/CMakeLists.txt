cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_depester)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (DEPESTER_SRCS
	depester.cpp
	)
set (DEPESTER_HEADERS
	depester.h
	)
set (DEPESTER_RESOURCES
	depesterresources.qrc
	)

QT4_ADD_RESOURCES (DEPESTER_RCCS ${DEPESTER_RESOURCES})
CreateTrs ("azoth_depester" "en;es;ru_RU;uk_UA" DEPESTER_COMPILED_TRANSLATIONS)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (DEPESTER_MOC_SRCS ${DEPESTER_HEADERS})
endif (NOT LC_NO_MOC)

add_library (leechcraft_azoth_depester SHARED
	${DEPESTER_COMPILED_TRANSLATIONS}
	${DEPESTER_SRCS}
	${DEPESTER_MOC_SRCS}
	${DEPESTER_UIS_H}
	${DEPESTER_RCCS}
	)
target_link_libraries (leechcraft_azoth_depester
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_depester DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${DEPESTER_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
