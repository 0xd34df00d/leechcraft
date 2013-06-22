cmake_minimum_required (VERSION 2.8)
project (leechcraft_blasq)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	blasq.cpp
	xmlsettingsmanager.cpp
	)
#CreateTrs("blasq" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("blasq" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_blasq SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_blasq
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_blasq DESTINATION ${LC_PLUGINS_DEST})
install (FILES blasqsettings.xml DESTINATION ${LC_SETTINGS_DEST})

option (ENABLE_BLASQ_SPEGNERSI "Build Flickr client module for Blasq" ON)

if (ENABLE_BLASQ_SPEGNERSI)
	add_subdirectory (plugins/spegnersi)
endif ()
