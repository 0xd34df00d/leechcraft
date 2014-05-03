if (NOT USE_QT5)
	set (QT_USE_QTDECLARATIVE TRUE)
	set (QT_USE_QTNETWORK TRUE)
	set (QT_USE_QTSQL TRUE)
	set (QT_USE_QTWEBKIT TRUE)
	set (QT_USE_QTXML TRUE)
	include (${QT_USE_FILE})
endif ()

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option (WITH_X11 "Enable X11 support on Linux" ON)
option (WITH_QWT "Enable support for Qwt (for QML PlotItem, for example)" ON)

include_directories (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${QWT_INCLUDE_DIRS}
	..)

add_subdirectory (models)
add_subdirectory (network)
add_subdirectory (qml)
add_subdirectory (shortcuts)
add_subdirectory (sll)
add_subdirectory (svcauth)
add_subdirectory (sys)
add_subdirectory (tags)
add_subdirectory (xdg)

set (SRCS
	gui/autoresizemixin.cpp
	gui/clearlineeditaddon.cpp
	gui/colorbutton.cpp
	gui/findnotification.cpp
	gui/findnotificationwk.cpp
	gui/flowlayout.cpp
	gui/lineeditbuttonmanager.cpp
	gui/pagenotification.cpp
	gui/selectablebrowser.cpp
	gui/unhoverdeletemixin.cpp
	gui/util.cpp
	xpc/basehookinterconnector.cpp
	xpc/defaulthookproxy.cpp
	xpc/notificationactionhandler.cpp
	xpc/passutils.cpp
	xpc/stdanfields.cpp
	xpc/stddatafiltermenucreator.cpp
	xpc/util.cpp
	xsd/util.cpp
	util.cpp
	dblock.cpp
	backendselector.cpp
	structuresops.cpp
	exceptions.cpp
	)
set (FORMS
	backendselector.ui
	gui/findnotification.ui
	)
if (USE_QT5)
	qt5_wrap_ui (UIS_H ${FORMS})
else ()
	QT4_WRAP_UI (UIS_H ${FORMS})
endif ()

if (WITH_X11 AND NOT APPLE)
	find_package (X11)

	if (X11_FOUND)
		add_definitions (-DHAVE_X11)
		set (ADDITIONAL_LIBS ${ADDITIONAL_LIBS} ${X11_X11_LIB})

		set (SRCS ${SRCS} x11/xwrapper.cpp)
	endif ()
endif ()

foreach (SRC ${SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/${SRC}")
endforeach ()
foreach (FORM ${FORMS})
	set (UTIL_FORMS "${UTIL_FORMS};util/${FORM}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)
set (UTIL_FORMS ${UTIL_FORMS} PARENT_SCOPE)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_NO_DEBUG)
add_definitions(-DQT_SHARED)
add_definitions(-DQT_GUI_LIB)

add_library (leechcraft-util SHARED
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft-util
	${QT_LIBRARIES}
	${ADDITIONAL_LIBS}
	leechcraft-xsd
	)
set_property (TARGET leechcraft-util PROPERTY SOVERSION 0.6.65)
install (TARGETS leechcraft-util DESTINATION ${LIBDIR})

if (USE_QT5)
	QT5_USE_MODULES (leechcraft-util Xml Sql Widgets)
endif ()
