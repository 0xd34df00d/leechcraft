cmake_minimum_required (VERSION 2.8)
project (leechcraft_laure)
include (InitLCPlugin OPTIONAL)

if (NOT MSVC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif (NOT MSVC)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option (TESTS_LAURE "Enable Laure tests" OFF)

set (QT_USE_QTGUI TRUE)
set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)

if (TESTS_LAURE)
	enable_testing ()
	set (QT_USE_QTTEST TRUE)
endif (TESTS_LAURE)

find_package (VLC REQUIRED)

if (NOT MSVC)
	find_package (MAGIC REQUIRED)
	if (MAGIC_FOUND)
		add_definitions (-DHAVE_MAGIC)
	endif (MAGIC_FOUND)
endif (NOT MSVC)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${VLC_INCLUDE_DIRS}
	${MAGIC_INCLUDE_DIR}
	)

set (SRCS
	positionslider.cpp
	core.cpp
	playliststatusdelegate.cpp
	separateplayer.cpp
	playbackmodemenu.cpp
	nowplayingdelegate.cpp
	volumeslider.cpp
	vlcwrapper.cpp
	playlistaddmenu.cpp
	playlistview.cpp
	xmlsettingsmanager.cpp
	playlistwidget.cpp
	chooseurldialog.cpp
	playpauseaction.cpp
	player.cpp
	laurewidget.cpp
	laure.cpp
	)
set (HEADERS
	positionslider.h
	core.h
	playliststatusdelegate.h
	separateplayer.h
	playbackmodemenu.h
	nowplayingdelegate.h
	volumeslider.h
	vlcwrapper.h
	playlistaddmenu.h
	playlistview.h
	xmlsettingsmanager.h
	playlistwidget.h
	chooseurldialog.h
	playpauseaction.h
	player.h
	laurewidget.h
	laure.h
	)
set (FORMS
	laurewidget.ui
	chooseurldialog.ui
)

set (RESOURCES laureresources.qrc)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif (NOT LC_NO_MOC)
QT4_WRAP_UI (FORMS_HEADERS ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})
CreateTrs ("laure" "en;ru_RU;uk_UA" COMPILED_TRANSLATIONS)

add_library (leechcraft_laure SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${HEADERS}
	${MOC_SRCS}
	${FORMS_HEADERS}
	${RCCS}
	)
target_link_libraries (leechcraft_laure
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${VLC_LIBRARIES}
	${MAGIC_LIBRARIES}
	)

install (TARGETS leechcraft_laure DESTINATION ${LC_PLUGINS_DEST})
install (FILES lauresettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})

set (LAURE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if (TESTS_LAURE)
	add_subdirectory (tests)
endif (TESTS_LAURE)
