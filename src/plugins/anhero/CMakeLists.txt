cmake_minimum_required (VERSION 2.8)
project (leechcraft_anhero)
include (InitLCPlugin OPTIONAL)

find_package (KDE4 4.2.0)

if (NOT KDE4_FOUND)
	message (WARNING "KDE 4.2 or higher was not found, not building AnHero.")
	return ()
endif (NOT KDE4_FOUND)

include (${QT_USE_FILE})
include (KDE4Defaults)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${KDE4_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	anhero.cpp
	)
set (HEADERS
	anhero.h
	)
set (RESOURCES anheroresources.qrc)
CreateTrs ("anhero" "ar;de;en;es;fr;it;oc;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif (NOT LC_NO_MOC)
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_anhero SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_anhero
	${QT_LIBRARIES}
	${KDE4_KUTILS_LIBS}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_anhero DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
