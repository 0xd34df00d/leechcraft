cmake_minimum_required (VERSION 2.8)
project (leechcraft_dumbeep)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	dumbeep.cpp
	xmlsettingsmanager.cpp
	)

add_library (leechcraft_dumbeep SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_dumbeep
	${LEECHCRAFT_LIBRARIES}
	${QT_PHONON_LIBS}
	)
install (TARGETS leechcraft_dumbeep DESTINATION ${LC_PLUGINS_DEST})
install (FILES dumbeepsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_dumbeep Core)
