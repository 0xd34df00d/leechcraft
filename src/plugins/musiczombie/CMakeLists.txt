cmake_minimum_required (VERSION 2.8)
project (leechcraft_musiczombie)
include (InitLCPlugin OPTIONAL)

option (WITH_MUSICZOMBIE_CHROMAPRINT "Build support for Chromaprints in MusicZombie" ON)

if (WITH_MUSICZOMBIE_CHROMAPRINT)
	set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
	find_package (FFmpeg COMPONENTS AVCODEC AVFORMAT AVUTIL SWRESAMPLE REQUIRED)

	set (EXTRA_LIBRARIES
		${AVFORMAT_LIBRARIES}
		${AVCODEC_LIBRARIES}
		${AVUTIL_LIBRARIES}
		${SWRESAMPLE_LIBRARIES}
		chromaprint)

	add_definitions (-DWITH_CHROMAPRINT)
endif ()

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	musiczombie.cpp
	artistlookup.cpp
	pendingdisco.cpp
	)
if (WITH_MUSICZOMBIE_CHROMAPRINT)
	set (SRCS
		${SRCS}
		chroma.cpp
		pendingtagsfetch.cpp)
endif ()

add_library (leechcraft_musiczombie SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
target_link_libraries (leechcraft_musiczombie
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${EXTRA_LIBRARIES}
	)
install (TARGETS leechcraft_musiczombie DESTINATION ${LC_PLUGINS_DEST})
