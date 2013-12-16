cmake_minimum_required (VERSION 2.8)
project (leechcraft_deadlyrics)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	deadlyrics.cpp
	searcher.cpp
	lyricscache.cpp
	sitessearcher.cpp
	concretesite.cpp
	)
CreateTrs ("deadlyrics" "ar;de;en;eo;es;fr;it;oc;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("deadlyrics" "en;ru_RU" "${SRCS}" "" "")
set (RESOURCES deadlyricsresources.qrc)

QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_deadlyrics SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_deadlyrics
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_deadlyrics DESTINATION ${LC_PLUGINS_DEST})
