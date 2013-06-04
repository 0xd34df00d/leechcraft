cmake_minimum_required (VERSION 2.8)
project (leechcraft_nacheku)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	nacheku.cpp
	xmlsettingsmanager.cpp
	clipboardwatcher.cpp
	directorywatcher.cpp
	)
set (HEADERS
	nacheku.h
	xmlsettingsmanager.h
	clipboardwatcher.h
	directorywatcher.h
	)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif (NOT LC_NO_MOC)

add_library (leechcraft_nacheku SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
target_link_libraries (leechcraft_nacheku
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_nacheku DESTINATION ${LC_PLUGINS_DEST})
install (FILES nachekusettings.xml DESTINATION ${LC_SETTINGS_DEST})
