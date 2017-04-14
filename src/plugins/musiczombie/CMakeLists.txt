cmake_minimum_required (VERSION 2.8)
project (leechcraft_musiczombie)
include (InitLCPlugin OPTIONAL)
include (CMakeDependentOption)

cmake_dependent_option (WITH_MUSICZOMBIE_CHROMAPRINT "Build support for Chromaprints in MusicZombie" ON "NOT WIN32" OFF)

if (WITH_MUSICZOMBIE_CHROMAPRINT)
	set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
	find_package (FFmpeg COMPONENTS AVCODEC AVFORMAT AVUTIL SWRESAMPLE REQUIRED)
	find_package (Chromaprint REQUIRED)

	set (EXTRA_LIBRARIES
		${AVFORMAT_LIBRARIES}
		${AVCODEC_LIBRARIES}
		${AVUTIL_LIBRARIES}
		${SWRESAMPLE_LIBRARIES}
		${CHROMAPRINT_LIBRARIES})

	include_directories (${CHROMAPRINT_INCLUDE_DIR} ${FFMPEG_INCLUDE_DIRS})

	add_definitions (-DWITH_CHROMAPRINT)
endif ()

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	musiczombie.cpp
	artistlookup.cpp
	pendingdisco.cpp
	util.cpp
	)
if (WITH_MUSICZOMBIE_CHROMAPRINT)
	set (SRCS
		${SRCS}
		chroma.cpp
		pendingtagsfetch.cpp)
endif ()

CreateTrs ("musiczombie" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("musiczombie" "en;ru_RU" "${SRCS}" "" "")

add_library (leechcraft_musiczombie SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_musiczombie
	${LEECHCRAFT_LIBRARIES}
	${EXTRA_LIBRARIES}
	)
install (TARGETS leechcraft_musiczombie DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_musiczombie Concurrent Gui Network Xml)
