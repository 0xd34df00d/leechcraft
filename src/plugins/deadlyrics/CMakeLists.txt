cmake_minimum_required (VERSION 2.8)
project (leechcraft_deadlyrics)
include (InitLCPlugin OPTIONAL)

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
set (HEADERS
	deadlyrics.h
	searcher.h
	lyricscache.h
	sitessearcher.h
	concretesite.h
	)
CreateTrs ("deadlyrics" "ar;de;en;eo;es;fr;it;oc;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
set (RESOURCES deadlyricsresources.qrc)

QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_deadlyrics SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)

FindQtLibs (leechcraft_deadlyrics Network Xml)

target_link_libraries (leechcraft_deadlyrics
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_deadlyrics DESTINATION ${LC_PLUGINS_DEST})
