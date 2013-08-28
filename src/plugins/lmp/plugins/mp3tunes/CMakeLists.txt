cmake_minimum_required (VERSION 2.8)
project (leechcraft_lmp_mp3tunes)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (${LMP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (MP3TUNES_SRCS
	mp3tunes.cpp
	accountsmanager.cpp
	authmanager.cpp
	playlistmanager.cpp
	uploader.cpp
	xmlsettingsmanager.cpp
	)

add_library (leechcraft_lmp_mp3tunes SHARED
	${MP3TUNES_COMPILED_TRANSLATIONS}
	${MP3TUNES_SRCS}
	)
target_link_libraries (leechcraft_lmp_mp3tunes
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_lmp_mp3tunes RUNTIME DESTINATION ${LC_PLUGINS_DEST})
install (FILES lmpmp3tunessettings.xml DESTINATION ${LC_SETTINGS_DEST})
