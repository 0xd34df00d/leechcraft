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
set (HEADERS
	deadlyrics.h
	searcher.h
	lyricscache.h
	sitessearcher.h
	concretesite.h
	)
CreateTrs ("deadlyrics" "ar;de;en;eo;es;fr;it;oc;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
set (RESOURCES deadlyricsresources.qrc)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif ()
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_deadlyrics SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_deadlyrics
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_deadlyrics RUNTIME DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
