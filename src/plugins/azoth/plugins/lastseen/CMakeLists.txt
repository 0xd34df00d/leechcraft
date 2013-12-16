cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_lastseen)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (LASTSEEN_SRCS
	lastseen.cpp
	)
set (LASTSEEN_RESOURCES
	lastseenresources.qrc
	)

QT4_ADD_RESOURCES (LASTSEEN_RCCS ${LASTSEEN_RESOURCES})

CreateTrs ("azoth_lastseen" "en;es;ru_RU;uk_UA" LASTSEEN_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_lastseen" "en;ru_RU" "${LASTSEEN_SRCS}" "" "")

add_library (leechcraft_azoth_lastseen SHARED
	${LASTSEEN_COMPILED_TRANSLATIONS}
	${LASTSEEN_SRCS}
	${LASTSEEN_RCCS}
	)
target_link_libraries (leechcraft_azoth_lastseen
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_lastseen DESTINATION ${LC_PLUGINS_DEST})
