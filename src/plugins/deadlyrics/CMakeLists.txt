cmake_minimum_required (VERSION 3.3)
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
	hascirylsearcher.cpp
	)
CreateTrs ("deadlyrics" "ar;de;en;eo;es;fr;it;oc;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("deadlyrics" "en;ru_RU" "${SRCS}" "" "")
set (RESOURCES deadlyricsresources.qrc)

QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_deadlyrics SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_deadlyrics
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_deadlyrics DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_deadlyrics Gui Network Xml)
